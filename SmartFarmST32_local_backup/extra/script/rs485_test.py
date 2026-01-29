# 라즈베리파이에 저장하고 라즈베리파이에서 실행.

import serial
import struct
import time
import sys
import argparse

# --- Constants from protocol (and reg_test.py) ---
START = 0x7E

# Common Commands
CMD_NODE_SELECT_REQ  = 0x20
CMD_NODE_SELECT_RESP = 0x21
CMD_FIRMWARE_VERSION_READ = 0xA0
CMD_FIRMWARE_VERSION_RESPONSE = 0xA1

CMD_REG_READ         = 0xB0 # For register access (used in reg_test.py)
CMD_REG_READ_RESP    = 0xB1
CMD_REG_WRITE        = 0xB2
CMD_REQUEST_FAILED   = 0x23
CMD_REQUEST_SUCCESS  = 0x24

# --- Utility Functions (Adapted from reg_test.py for simplicity) ---
def ts() -> str:
    t = time.time()
    lt = time.localtime(t)
    ms = int((t - int(t)) * 1000)
    return time.strftime("%H:%M:%S", lt) + f".{ms:03d}"

def hexdump(b: bytes, width=16) -> str:
    if b is None:
        return "<None>"
    out = []
    for i in range(0, len(b), width):
        chunk = b[i:i+width]
        hexpart = " ".join(f"{x:02X}" for x in chunk)
        asc = "".join(chr(x) if 32 <= x <= 126 else "." for x in chunk)
        out.append(f"{i:04X}  {hexpart:<{width*3}}  {asc}")
    return "\n".join(out) if out else "<empty>"

def xor_checksum(buf: bytes) -> int:
    c = 0
    for b in buf:
        c ^= b
    return c & 0xFF

def make_packet(cmd: int, data: bytes = None) -> bytes:
    if data is None:
        data = b""
    hdr = bytes([START, cmd & 0xFF, len(data) & 0xFF])
    body = hdr + data
    chk = xor_checksum(body)
    return body + bytes([chk])

def read_exact(ser: serial.Serial, n: int, timeout_s=1.0, inter_gap_s=0.0, log_prefix="") -> bytes:
    t0 = time.time()
    out = b""
    while len(out) < n:
        if (time.time() - t0) > timeout_s:
            break
        chunk = ser.read(n - len(out))
        if chunk:
            out += chunk
            if inter_gap_s > 0:
                time.sleep(inter_gap_s)
        else:
            time.sleep(0.001)
    return out

def drain(ser: serial.Serial, label="DRAIN"):
    try:
        waiting = ser.in_waiting
    except Exception:
        waiting = 0
    if waiting and waiting > 0:
        data = ser.read(waiting)
        print(f"[{ts()}] [{label}] in_waiting={waiting} bytes:")
        print(hexdump(data))
        return data
    return b""

def read_packet(ser: serial.Serial, timeout_s=1.0, inter_gap_s=0.0, verbose=False):
    t0 = time.time()
    skipped = 0
    while True:
        if (time.time() - t0) > timeout_s:
            if verbose:
                print(f"[{ts()}] [RX] timeout waiting START(0x7E). skipped={skipped}")
            return None
        b = ser.read(1)
        if not b:
            time.sleep(0.001)
            continue
        if b[0] == START:
            if verbose:
                print(f"[{ts()}] [RX] START found. skipped_before_start={skipped}")
            break
        skipped += 1
    hdr_rest = read_exact(ser, 2, timeout_s=timeout_s, inter_gap_s=inter_gap_s, log_prefix="[RX]" if verbose else "")
    if len(hdr_rest) != 2:
        return None
    cmd = hdr_rest[0]
    dlen = hdr_rest[1]
    data = read_exact(ser, dlen, timeout_s=timeout_s, inter_gap_s=inter_gap_s, log_prefix="[RX]" if verbose else "")
    if len(data) != dlen:
        return None
    chk_b = read_exact(ser, 1, timeout_s=timeout_s, inter_gap_s=inter_gap_s, log_prefix="[RX]" if verbose else "")
    if len(chk_b) != 1:
        return None
    chk = chk_b[0]
    raw_wo_chk = bytes([START]) + hdr_rest + data
    calc = xor_checksum(raw_wo_chk)
    if calc != chk:
        return ("bad_checksum", cmd, dlen, data, chk, calc)
    return ("ok", cmd, dlen, data, chk)

def tx(ser: serial.Serial, pkt: bytes, verbose=False, label="TX"):
    if verbose:
        print(f"[{ts()}] [{label}] write {len(pkt)} bytes:\n{hexdump(pkt)}")
    n = ser.write(pkt)
    ser.flush()
    if verbose:
        print(f"[{ts()}] [{label}] wrote={n} bytes (flush done)")
    return n

# --- Specific Test Functions for rs485_test.py ---
def node_select_transaction(ser: serial.Serial, serial_number_u64: int, timeout_s=1.0, verbose=False) -> bool:
    payload = struct.pack("<Q", serial_number_u64)
    pkt = make_packet(CMD_NODE_SELECT_REQ, payload)
    
    drain(ser, "PRE_NODE_SELECT_DRAIN")
    tx(ser, pkt, verbose=verbose, label="NODE_SELECT_TX")

    resp = read_packet(ser, timeout_s=timeout_s, verbose=verbose)
    if resp is None:
        print(f"[{ts()}] [NODE_SELECT] No response.")
        return False
    if resp[0] == "bad_checksum":
        print(f"[{ts()}] [NODE_SELECT] Checksum error: {resp}")
        return False

    _, cmd, dlen, data, _chk = resp
    if cmd == CMD_NODE_SELECT_RESP and dlen == 1 and data[0] == 0x01:
        print(f"[{ts()}] [NODE_SELECT] Node selected successfully.")
        return True
    else:
        print(f"[{ts()}] [NODE_SELECT] Unexpected response: cmd=0x{cmd:02X}, dlen={dlen}, data={data.hex()}")
        return False

def read_firmware_version(ser: serial.Serial, timeout_s=1.0, verbose=False):
    pkt = make_packet(CMD_FIRMWARE_VERSION_READ) # No data for this command

    drain(ser, "PRE_FIRMWARE_VERSION_DRAIN")
    tx(ser, pkt, verbose=verbose, label="FIRMWARE_VERSION_TX")

    resp = read_packet(ser, timeout_s=timeout_s, verbose=verbose)
    if resp is None:
        print(f"[{ts()}] [FIRMWARE_VERSION] No response.")
        return None
    if resp[0] == "bad_checksum":
        print(f"[{ts()}] [FIRMWARE_VERSION] Checksum error: {resp}")
        return None

    _, cmd, dlen, data, _chk = resp
    if cmd == CMD_FIRMWARE_VERSION_RESPONSE:
        try:
            version_str = data.decode('ascii').strip()
            print(f"[{ts()}] [FIRMWARE_VERSION] Received version: {version_str}")
            return version_str
        except UnicodeDecodeError:
            print(f"[{ts()}] [FIRMWARE_VERSION] Could not decode version data: {data.hex()}")
            return None
    elif cmd == CMD_REQUEST_FAILED:
        print(f"[{ts()}] [FIRMWARE_VERSION] Request failed response.")
        return None
    else:
        print(f"[{ts()}] [FIRMWARE_VERSION] Unexpected response: cmd=0x{cmd:02X}, dlen={dlen}, data={data.hex()}")
        return None

def reg_read_transaction(ser: serial.Serial, addr: int, width: int, timeout_s=1.0, verbose=False) -> bytes:
    # reg_test.py uses <HBB for payload struct, last B is reserved and 0
    payload = struct.pack("<HBB", addr & 0xFFFF, width & 0xFF, 0)
    pkt = make_packet(CMD_REG_READ, payload)

    drain(ser, "PRE_REG_READ_DRAIN")
    tx(ser, pkt, verbose=verbose, label="REG_READ_TX")

    resp = read_packet(ser, timeout_s=timeout_s, verbose=verbose)
    if resp is None:
        print(f"[{ts()}] [REG_READ] No response.")
        return None
    if resp[0] == "bad_checksum":
        print(f"[{ts()}] [REG_READ] Checksum error: {resp}")
        return None

    _, cmd, dlen, data, _chk = resp
    if cmd == CMD_REG_READ_RESP:
        if dlen < 3: # Must contain addr (H) and width (B) and at least 0 bytes of value
            print(f"[{ts()}] [REG_READ] Invalid payload length: {dlen}. Expected at least 3 bytes (Addr+Width+Value).")
            return None
        r_addr, r_width = struct.unpack_from("<HB", data, 0)
        value_data = data[3:]
        if r_addr == (addr & 0xFFFF) and r_width == width and len(value_data) == width:
            print(f"[{ts()}] [REG_READ] OK. Addr: 0x{r_addr:04X}, Width: {r_width}, Value: {value_data.hex()}")
            return value_data
        else:
            print(f"[{ts()}] [REG_READ] Mismatch in response. Expected Addr: 0x{addr:04X}, Width: {width}. Got Addr: 0x{r_addr:04X}, Width: {r_width}, Data Len: {len(value_data)}. Raw data: {data.hex()}")
            return None
    elif cmd == CMD_REQUEST_FAILED:
        print(f"[{ts()}] [REG_READ] Request failed response.")
        return None
    else:
        print(f"[{ts()}] [REG_READ] Unexpected response: cmd=0x{cmd:02X}, dlen={dlen}, data={data.hex()}")
        return None

def main():
    p = argparse.ArgumentParser(description="RS485 communication test and MCU setup.")
    p.add_argument("port", help="Serial port (e.g., /dev/ttyS0 or COMx)")
    p.add_argument("baud", type=int, default=19200, help="Baud rate (default: 19200)")
    p.add_argument("serial_u64_hex", help="64-bit Node Serial Number (e.g., 0x1122334455667788)")
    p.add_argument("--timeout", type=float, default=2.0, help="Packet read timeout in seconds (default: 2.0)")
    p.add_argument("--verbose", action="store_true", help="Print verbose diagnostics")
    p.add_argument("--read_version", action="store_true", help="Read firmware version of the node")
    p.add_argument("--read_fan_status", action="store_true", help="Read fan power enable status (Reg 0x0160)")

    args = p.parse_args()

    sn = int(args.serial_u64_hex, 16)

    ser = serial.Serial(
        port=args.port,
        baudrate=args.baud,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=args.timeout,
    )

    ser.reset_input_buffer()
    ser.reset_output_buffer()

    print(f"[{ts()}] Opened {args.port} @ {args.baud} 8N1 (timeout={ser.timeout}s)")
    print(f"[{ts()}] Target Node SN: 0x{sn:016X}")

    try:
        # --- 1. Node Select ---
        if node_select_transaction(ser, sn, timeout_s=args.timeout, verbose=args.verbose):
            # --- 2. Perform actions based on arguments ---
            if args.read_version:
                print(f"[{ts()}] Attempting to read firmware version...")
                read_firmware_version(ser, timeout_s=args.timeout, verbose=args.verbose)

            if args.read_fan_status:
                print(f"[{ts()}] Attempting to read Fan Power Enable (Reg 0x0160)...")
                fan_status_data = reg_read_transaction(ser, 0x0160, 1, timeout_s=args.timeout, verbose=args.verbose)
                if fan_status_data:
                    fan_on = struct.unpack("<B", fan_status_data)[0]
                    print(f"[{ts()}] [REG_READ] FAN_POWER_ENABLE (0x0160) = {fan_on} (0=OFF, 1=ON)")
            
            if not (args.read_version or args.read_fan_status):
                print(f"[{ts()}] No specific action requested. Node selected successfully.")
                print(f"[{ts()}] Use --read_version or --read_fan_status to perform actions.")

        else:
            print(f"[{ts()}] Node selection failed. Please check serial number, wiring, power, and MCU firmware.")

    except serial.SerialException as e:
        print(f"[{ts()}] Serial Port Error: {e}")
    except Exception as e:
        print(f"[{ts()}] An unexpected error occurred: {e}")
    finally:
        if ser.is_open:
            ser.close()
            print(f"[{ts()}] Serial port closed.")

if __name__ == "__main__":
    main()
