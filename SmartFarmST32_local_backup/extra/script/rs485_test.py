# 라즈베리파이에 저장하고 라즈베리파이에서 실행.
# 실행 명령어 : python3 rs485_test.py /dev/ttyUSB0 19200 0x4653500D004C003C --read_version --read_fan_status --verbose
# 테스트용 팬 제어를 포함한 명령어 Fan off
# : python3 rs485_test.py /dev/ttyUSB0 19200 0x4653500D004C003C --write 0x0160:1=0 --verbose
# 테스트용 팬 제어를 포함한 명령어 Fan on
# : python3 rs485_test.py /dev/ttyUSB0 19200 0x4653500D004C003C --write 0x0160:1=1 --verbose

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

# Analog Commands
CMD_ANALOG_READ_ALL = 0x51
CMD_ANALOG_ALL_RESPONSE = 0x61

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
    
def read_analog_all_transaction(ser: serial.Serial, timeout_s=1.0, verbose=False) -> list:
    # Read all analog voltage and current values from the sensor node.
    pkt = make_packet(CMD_ANALOG_READ_ALL)

    drain(ser, "PRE_ANALOG_READ_ALL_DRAIN")
    tx(ser, pkt, verbose=verbose, label="ANALOG_READ_ALL_TX")

    resp = read_packet(ser, timeout_s=timeout_s, verbose=verbose)
    if resp is None:
        print(f"[{ts()}] [ANALOG_READ_ALL] No response received (timeout={timeout_s}s).")
        return None
    if resp[0] == "bad_checksum":
        print(f"[{ts()}] [ANALOG_READ_ALL] Checksum error: {resp}")
        return None
    
    _, cmd, dlen, data, _chk = resp
    if cmd == CMD_ANALOG_ALL_RESPONSE:
        expected_dlen = 16
        if dlen != expected_dlen:
            print(f"[{ts()}] [ANALOG_READ_ALL] Unexpected response data length: {dlen}. Expected {expected_dlen}.")
            return None
        
        results = []

        for i in range(4):
            voltage_mV = struct.unpack_from("<h", data, i * 2)[0]
            current_uA = struct.unpack_from("<H", data, (4 * 2) + i * 2)[0]
            results.append({'channel': i, 'voltage_mV': voltage_mV, 'current_uA': current_uA})
            print(f"[{ts()}] [ANALOG_READ_ALL] CH{i+1}: Voltage={voltage_mV}mV, Current={current_uA}uA")
        return results
    elif cmd == CMD_REQUEST_FAILED:
        print(f"[{ts()}] [ANALOG_READ_ALL] Request failed response received.")
        return None
    else:
        print(f"[{ts()}] [ANALOG_READ_ALL] Unexpected response: cmd=0x{cmd:02X}, dlen={dlen}, data={data.hex()}")
        return None

def reg_write_transaction(ser: serial.Serial, addr: int, width: int, value: bytes, timeout_s=1.0, verbose=False):
    if value is None:
        value = b""
    if len(value) != width:
        print(f"[{ts()}] [REG_WRITE] Value length {len(value)} != width {width}")
        return False
    
    payload = struct.pack("<HB", addr & 0xFFFF, width & 0xFF) + value
    pkt = make_packet(CMD_REG_WRITE, payload)

    drain(ser, "PRE_REG_WRITE_DRAIN")
    tx(ser, pkt, verbose=verbose, label="REG_WRITE_TX")

    resp = read_packet(ser, timeout_s=timeout_s, verbose=verbose)
    if resp is None:
        print(f"[{ts()}] [REG_WRITE] No response (timeout={timeout_s}s)")
        return False
    if resp[0] == "bad_checksum":
        print(f"[{ts()}] [REG_WRITE] Checksum error: {resp}")
        return False
    
    _, cmd, dlen, data, _chk = resp
    if dlen != 0:
        print(f"[{ts()}] [REG_WRITE] Unexpected response payload length {dlen}, data={data.hex()}")
        return False
    if cmd == CMD_REQUEST_SUCCESS:
        print(f"[{ts()}] [REG_WRITE] Request SUCCESS.")
        return True
    if cmd == CMD_REQUEST_FAILED:
        print(f"[{ts()}] [REG_WRITE] Reques FAILED.")
        return False

    print(f"[{ts()}] [REG_WRITE] Unexpected response cmd=0x{cmd:02X}, len={dlen}, data={data.hex()}")
    return False

def parse_addr_width(s: str):
    if ":" in s:
        addr_s, width_s = s.split(":", 1)
        addr = int(addr_s, 0)
        width = int(width_s, 0)
        return addr, width
    addr = int(s, 0)
    raise ValueError(f"width required for addr 0x{addr:04X} (use addr:width)")

def parse_write_spec(s: str):
    if "=" not in s:
        raise ValueError("write spec must be addr[:width]=value")
    left, value_s = s.split("=", 1)
    addr, width = parse_addr_width(left)
    value_int = int(value_s, 0)
    max_val = (1 << (width * 8)) - 1
    if value_int < 0 or value_int > max_val:
        raise ValueError(f"value out of range for width {width}: {value_int}")
    value = value_int.to_bytes(width, byteorder="little", signed=False)
    return addr, width, value_int, value


def main():
    p = argparse.ArgumentParser(description="RS485 communication test and MCU setup.")
    p.add_argument("port", help="Serial port (e.g., /dev/ttyS0 or COMx)")
    p.add_argument("baud", type=int, default=19200, help="Baud rate (default: 19200)")
    p.add_argument("serial_u64_hex", help="64-bit Node Serial Number (e.g., 0x1122334455667788)")
    p.add_argument("--timeout", type=float, default=2.0, help="Packet read timeout in seconds (default: 2.0)")
    p.add_argument("--verbose", action="store_true", help="Print verbose diagnostics")
    p.add_argument("--read_version", action="store_true", help="Read firmware version of the node")
    p.add_argument("--read_fan_status", action="store_true", help="Read fan power enable status (Reg 0x0160)")
    p.add_argument("--read_analog_all", action="store_true", help="Read all analog voltage and current values")
    p.add_argument("--write", action="append", default=[], metavar="ADDR:WIDTH=VALUE", help="Write register (e.g. 0x0160:1=1). Can repeat. (ADDR:WIDTH=VALUE)")


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
                if node_select_transaction(ser, sn, timeout_s=args.timeout, verbose=args.verbose):
                    read_firmware_version(ser, timeout_s=args.timeout, verbose=args.verbose)
                else:
                    print(f"[{ts()}] Node re-selection failed before reading firmware version.")

            if args.read_fan_status:
                print(f"[{ts()}] Attempting to read Fan Power Enable (Reg 0x0160)...")
                if node_select_transaction(ser, sn, timeout_s=args.timeout, verbose=args.verbose):
                    fan_status_data = reg_read_transaction(ser, 0x0160, 1, timeout_s=args.timeout, verbose=args.verbose)
                    if fan_status_data:
                        fan_on = struct.unpack("<B", fan_status_data)[0]
                        print(f"[{ts()}] [REG_READ] FAN_POWER_ENABLE (0x0160) = {fan_on} (0=OFF, 1=ON)")
                else:
                    print(f"[{ts()}] Node re-selection failed before reading fan status.")
            
            # Logic to handle --read_analog_all argument
            if args.read_analog_all:
                print(f"[{ts()}] Attempting to read all analog values...")
                if node_select_transaction(ser, sn, timeout_s=args.timeout, verbose=args.verbose):
                    analog_data = read_analog_all_transaction(ser, timeout_s=args.timeout, verbose=args.verbose)
                    if analog_data:
                        print(f"[{ts()}] [ANALOG_READ_ALL] Successfully received analog data for all channels.")
                    else:
                        print(f"[{ts()}] [ANALOG_READ_ALL] Failed to read analog data.")
                else:
                    print(f"[{ts()}] Node re-selection failed before reading all analog values.")

            for spec in args.write:
                try:
                    addr, width, value_int, value = parse_write_spec(spec)
                except ValueError as e:
                    print(f"[{ts()}] WRITE parse error: {e}")
                    sys.exit(2)

                print(f"[{ts()}] Attempting to write REG 0x{addr:04X} with value {value_int}...")
                if node_select_transaction(ser, sn, timeout_s=args.timeout, verbose=args.verbose):
                    success = reg_write_transaction(
                        ser, addr, width, value, timeout_s=args.timeout, verbose=args.verbose)
                    print(f"[{ts()}] REG 0x{addr:04X} WRITE = {value_int} ({'OK' if success else 'FAIL'})")
                else:
                    print(f"[{ts()}] Node re-selection failed before writing REG 0x{addr:04X}.")

            if not (args.read_version or args.read_fan_status or args.read_analog_all or args.write):
                print(f"[{ts()}] No specific action requested. Node selected successfully.")
                print(f"[{ts()}] Use --read_version or --read_fan_status, --read_analog_all, or --write to perform actions.")

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
