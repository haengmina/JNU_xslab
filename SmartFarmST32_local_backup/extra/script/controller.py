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

CMD_REG_READ         = 0xB0 # For register access
CMD_REG_READ_RESP    = 0xB1
CMD_REQUEST_FAILED   = 0x23
CMD_REQUEST_SUCCESS  = 0x24

# --- Utility Functions (Adapted from rs485_test.py) ---
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
        if verbose: print(f"[{ts()}] [NODE_SELECT] No response.")
        return False
    if resp[0] == "bad_checksum":
        if verbose: print(f"[{ts()}] [NODE_SELECT] Checksum error: {resp}")
        return False

    _, cmd, dlen, data, _chk = resp
    if cmd == CMD_NODE_SELECT_RESP and dlen == 1 and data[0] == 0x01:
        if verbose: print(f"[{ts()}] [NODE_SELECT] Node selected successfully.")
        return True
    else:
        if verbose: print(f"[{ts()}] [NODE_SELECT] Unexpected response: cmd=0x{cmd:02X}, dlen={dlen}, data={data.hex()}")
        return False

def reg_read_transaction(ser: serial.Serial, addr: int, width: int, timeout_s=1.0, verbose=False) -> bytes:
    payload = struct.pack("<HBB", addr & 0xFFFF, width & 0xFF, 0) # addr (H), width (B), reserved (B)
    pkt = make_packet(CMD_REG_READ, payload)

    drain(ser, "PRE_REG_READ_DRAIN")
    tx(ser, pkt, verbose=verbose, label="REG_READ_TX")

    resp = read_packet(ser, timeout_s=timeout_s, verbose=verbose)
    if resp is None:
        raise RuntimeError(f"REG_READ (0x{addr:04X}): No response (timeout={timeout_s}s)")
    if resp[0] == "bad_checksum":
        raise RuntimeError(f"REG_READ (0x{addr:04X}): Checksum error: {resp}")

    _, cmd, dlen, data, _chk = resp
    if cmd == CMD_REG_READ_RESP:
        if dlen < 3: # Must contain addr (H) and width (B) and at least 0 bytes of value
            raise RuntimeError(f"REG_READ (0x{addr:04X}): Invalid payload length: {dlen}. Expected at leas 3 bytes (Addr+Width+Value).")
        r_addr, r_width = struct.unpack_from("<HB", data, 0)
        value_data = data[3:]
        if r_addr == (addr & 0xFFFF) and r_width == width and len(value_data) == width:
            if verbose: print(f"[{ts()}] [REG_READ] OK. Addr: 0x{r_addr:04X}, Width: {r_width}, Value: {value_data.hex()}")
            return value_data
        else:
            raise RuntimeError(
                f"REG_READ (0x{addr:04X}): Mismatch in response. Expected Addr: 0x{addr:04X}, Width: {width}. "
                f"Got Addr: 0x{r_addr:04X}, Width: {r_width}, Data Len: {len(value_data)}. Raw data: {data.hex()}"
            )
    elif cmd == CMD_REQUEST_FAILED:
        raise RuntimeError(f"REG_READ (0x{addr:04X}): Request failed response.")
    else:
        raise RuntimeError(f"REG_READ (0x{addr:04X}): Unexpected response: cmd=0x{cmd:02X}, dlen={dlen}, data={data.hex()}")

# --- Interpretation Functions ---
def interpret_temperature_c(raw_data: bytes) -> float:
    # Register 0x0200 (TEMPERATURE_C), 2 bytes, int16. Value is actual_temp * 10
    if len(raw_data) != 2:
        raise ValueError(f"Expected 2 bytes for temperature, got {len(raw_data)}")
    val_int10 = struct.unpack("<h", raw_data)[0] # <h for signed short, little-endian
    return val_int10 / 10.0

def interpret_humidity_rh(raw_data: bytes) -> float:
    # Register 0x0140 (HUMIDITY_%RH), 2 bytes, uint16. Value is actual_humidity * 10
    if len(raw_data) != 2:
        raise ValueError(f"Expected 2 bytes for humidity, got {len(raw_data)}")
    val_uint10 = struct.unpack("<H", raw_data)[0] # <H for unsigned short, little-endian
    return val_uint10 / 10.0
 
def interpret_fan_status(raw_data: bytes) -> str:
    # Register 0x0160 (FAN_POWER_ENABLE), 1 byte, uint8. 0=OFF, 1=ON
    if len(raw_data) != 1:
        raise ValueError(f"Expected 1 byte for fan status, got {len(raw_data)}")
    status = raw_data[0]
    return "ON" if status == 1 else "OFF"

def interpret_sensor_power_status(raw_data: bytes) -> str:
    # Register 0x0180 (SENSOR_POWER_STATE), 1 byte, uint8. 0=OFF, 1=ON
    if len(raw_data) != 1:
        raise ValueError(f"Expected 1 byte for sensor power status, got {len(raw_data)}")
    status = raw_data[0]
    return "ON" if status == 1 else "OFF"


def main():
    p = argparse.ArgumentParser(description="Collects sensor data from STM32 via RS485.")
    p.add_argument("port", help="Serial port (e.g., /dev/ttyUSB0)")
    p.add_argument("baud", type=int, default=19200, help="Baud rate (default: 19200)")
    p.add_argument("serial_u64_hex", help="64-bit Node Serial Number (e.g., 0x1122334455667788)")
    p.add_argument("--timeout", type=float, default=2.0, help="Packet read timeout in seconds (default: 2.0)")
    p.add_argument("--interval", type=float, default=5.0, help="Data collection interval in seconds (default: 5.0)")
    p.add_argument("--verbose", action="store_true", help="Print verbose diagnostics")
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
    print(f"[{ts()}] Data collection interval: {args.interval}s")

    try:
        while True:
            # Read Temperature (0x0200)
            try:
                if not node_select_transaction(ser, sn, timeout_s=args.timeout, verbose=args.verbose):
                    print(f"[{ts()}] Node select failed. Retrying in {args.interval}s...")
                    time.sleep(args.interval)
                    continue
                temp_raw = reg_read_transaction(ser, 0x0200, 2, timeout_s=args.timeout, verbose=args.verbose)
                temperature_c = interpret_temperature_c(temp_raw)
                print(f"[{ts()}] Temperature: {temperature_c:.1f} °C")
            except Exception as e:
                print(f"[{ts()}] Error reading temperature: {e}")

            # Read Humidity (0x0140)
            try:
                if not node_select_transaction(ser, sn, timeout_s=args.timeout, verbose=args.verbose):
                    print(f"[{ts()}] Node re-select failed. Retrying in {args.interval}s...")
                    time.sleep(args.interval)
                    continue
                hum_raw = reg_read_transaction(ser, 0x0140, 2, timeout_s=args.timeout, verbose=args.verbose)
                humidity_rh = interpret_humidity_rh(hum_raw)
                print(f"[{ts()}] Humidity: {humidity_rh:.1f} %RH")
            except Exception as e:
                print(f"[{ts()}] Error reading humidity: {e}")

            # Read Fan Status (0x0160)
            try:
                if not node_select_transaction(ser, sn, timeout_s=args.timeout, verbose=args.verbose):
                    print(f"[{ts()}] Node re-select failed. Retrying in {args.interval}s...")
                    time.sleep(args.interval)
                    continue
                fan_raw = reg_read_transaction(ser, 0x0160, 1, timeout_s=args.timeout, verbose=args.verbose)
                fan_status = interpret_fan_status(fan_raw)
                print(f"[{ts()}] Fan Status: {fan_status}")
            except Exception as e:
                print(f"[{ts()}] Error reading fan status: {e}")

            # Read Sensor Power Status (0x0180)
            try:
                if not node_select_transaction(ser, sn, timeout_s=args.timeout, verbose=args.verbose):
                    print(f"[{ts()}] Node re-select failed. Retrying in {args.interval}s...")
                    time.sleep(args.interval)
                    continue
                sensor_power_raw = reg_read_transaction(ser, 0x0180, 1, timeout_s=args.timeout, verbose=args.verbose)
                sensor_power_status = interpret_sensor_power_status(sensor_power_raw)
                print(f"[{ts()}] Sensor Power Status: {sensor_power_status}")
            except Exception as e:
                print(f"[{ts()}] Error reading sensor power status: {e}")

            print("-" * 30) # Separator for readability
            time.sleep(args.interval)

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