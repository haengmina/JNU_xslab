import serial
import struct
import time
import sys
import argparse

START = 0x7E

CMD_NODE_SELECT_REQ  = 0x20
CMD_NODE_SELECT_RESP = 0x21

CMD_REG_READ         = 0xB0
CMD_REG_READ_RESP    = 0xB1
CMD_REG_WRITE        = 0xB2
CMD_REQUEST_FAILED   = 0x23
CMD_REQUEST_SUCCESS  = 0x24

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

def make_packet(cmd: int, data: bytes) -> bytes:
    if data is None:
        data = b""
    hdr = bytes([START, cmd & 0xFF, len(data) & 0xFF])
    body = hdr + data
    chk = xor_checksum(body)
    return body + bytes([chk])

def infer_width(addr: int):
    if addr in (0x0100, 0x0120, 0x0140):
        return 2
    if addr == 0x0160:
        return 1
    return None

def parse_addr_width(s: str):
    if ":" in s:
        addr_s, width_s = s.split(":", 1)
        addr = int(addr_s, 0)
        width = int(width_s, 0)
        return addr, width
    addr = int(s, 0)
    width = infer_width(addr)
    if width is None:
        raise ValueError(f"width required for addr 0x{addr:04X} (use addr:width)")
    return addr, width

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

def read_exact(ser: serial.Serial, n: int, timeout_s=1.0, inter_gap_s=0.0, log_prefix="") -> bytes:
    """
    n bytes를 timeout_s 안에 모으기. 중간에 inter_gap_s만큼 쉼(옵션).
    """
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
    if log_prefix:
        print(f"[{ts()}] {log_prefix} read_exact({n}) -> {len(out)} bytes")
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
    """
    Packet format: [0]=0x7E [1]=cmd [2]=len [3..]=data [N]=checksum
    """
    t0 = time.time()

    # 1) START
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
        if verbose and skipped <= 16:
            print(f"[{ts()}] [RX] skip byte: 0x{b[0]:02X}")
        elif verbose and skipped == 17:
            print(f"[{ts()}] [RX] ... skipping more bytes (suppressing) ...")

    # 2) cmd + len
    hdr_rest = read_exact(ser, 2, timeout_s=timeout_s, inter_gap_s=inter_gap_s, log_prefix="[RX]" if verbose else "")
    if len(hdr_rest) != 2:
        if verbose:
            print(f"[{ts()}] [RX] header_rest short: got={len(hdr_rest)} expected=2")
            drain(ser, "RX_REMAIN_AFTER_HDR_SHORT")
        return None

    cmd = hdr_rest[0]
    dlen = hdr_rest[1]
    if verbose:
        print(f"[{ts()}] [RX] header cmd=0x{cmd:02X} dlen={dlen}")

    # 3) data
    data = read_exact(ser, dlen, timeout_s=timeout_s, inter_gap_s=inter_gap_s, log_prefix="[RX]" if verbose else "")
    if len(data) != dlen:
        if verbose:
            print(f"[{ts()}] [RX] data short: got={len(data)} expected={dlen}")
            if len(data) > 0:
                print(f"[{ts()}] [RX] partial data:\n{hexdump(data)}")
            drain(ser, "RX_REMAIN_AFTER_DATA_SHORT")
        return None

    # 4) checksum
    chk_b = read_exact(ser, 1, timeout_s=timeout_s, inter_gap_s=inter_gap_s, log_prefix="[RX]" if verbose else "")
    if len(chk_b) != 1:
        if verbose:
            print(f"[{ts()}] [RX] checksum short: got={len(chk_b)} expected=1")
            drain(ser, "RX_REMAIN_AFTER_CHK_SHORT")
        return None
    chk = chk_b[0]

    raw_wo_chk = bytes([START]) + hdr_rest + data
    calc = xor_checksum(raw_wo_chk)
    if calc != chk:
        if verbose:
            print(f"[{ts()}] [RX] BAD CHECKSUM: recv=0x{chk:02X} calc=0x{calc:02X}")
            raw_full = raw_wo_chk + bytes([chk])
            print(f"[{ts()}] [RX] raw packet:\n{hexdump(raw_full)}")
        return ("bad_checksum", cmd, dlen, data, chk, calc)

    if verbose:
        raw_full = raw_wo_chk + bytes([chk])
        print(f"[{ts()}] [RX] OK packet ({len(raw_full)} bytes):\n{hexdump(raw_full)}")

    return ("ok", cmd, dlen, data, chk)

def tx(ser: serial.Serial, pkt: bytes, verbose=False, label="TX"):
    if verbose:
        print(f"[{ts()}] [{label}] write {len(pkt)} bytes:\n{hexdump(pkt)}")
    n = ser.write(pkt)
    ser.flush()
    if verbose:
        print(f"[{ts()}] [{label}] wrote={n} bytes (flush done)")
    return n

def node_select(ser: serial.Serial, serial_number_u64: int, timeout_s=1.0, inter_gap_s=0.0, verbose=False, retries=1) -> bool:
    payload = struct.pack("<Q", serial_number_u64)
    pkt = make_packet(CMD_NODE_SELECT_REQ, payload)

    for attempt in range(1, retries + 1):
        if verbose:
            print(f"[{ts()}] [NODE_SELECT] attempt {attempt}/{retries} sn=0x{serial_number_u64:016X}")
        drain(ser, "PRE_TX_DRAIN")
        tx(ser, pkt, verbose=verbose, label="NODE_SELECT_TX")

        resp = read_packet(ser, timeout_s=timeout_s, inter_gap_s=inter_gap_s, verbose=verbose)
        if resp is None:
            print(f"[{ts()}] [NODE_SELECT] no response (timeout={timeout_s}s)")
            drain(ser, "POST_TIMEOUT_DRAIN")
            continue
        if resp[0] == "bad_checksum":
            print(f"[{ts()}] [NODE_SELECT] checksum error: {resp}")
            continue

        _, cmd, dlen, data, _chk = resp
        if cmd != CMD_NODE_SELECT_RESP or dlen != 1:
            print(f"[{ts()}] [NODE_SELECT] unexpected resp cmd=0x{cmd:02X} len={dlen} data={data.hex()}")
            continue

        status = data[0]
        print(f"[{ts()}] [NODE_SELECT] status=0x{status:02X}")
        return status == 0x01

    return False

def reg_read(ser: serial.Serial, addr: int, width: int, timeout_s=1.0, inter_gap_s=0.0, verbose=False) -> bytes:
    payload = struct.pack("<HBB", addr & 0xFFFF, width & 0xFF, 0)
    pkt = make_packet(CMD_REG_READ, payload)

    drain(ser, "PRE_TX_DRAIN")
    tx(ser, pkt, verbose=verbose, label="REG_READ_TX")

    resp = read_packet(ser, timeout_s=timeout_s, inter_gap_s=inter_gap_s, verbose=verbose)
    if resp is None:
        raise RuntimeError(f"REG_READ: no response (timeout={timeout_s}s)")
    if resp[0] == "bad_checksum":
        raise RuntimeError(f"REG_READ: checksum error: {resp}")

    _, cmd, dlen, data, _chk = resp
    if cmd != CMD_REG_READ_RESP:
        raise RuntimeError(f"REG_READ: unexpected cmd=0x{cmd:02X}, len={dlen}, data={data.hex()}")

    if dlen < 3:
        raise RuntimeError(f"REG_READ: invalid payload len={dlen}")

    r_addr, r_width = struct.unpack_from("<HB", data, 0)
    value = data[3:]
    if r_addr != (addr & 0xFFFF) or r_width != width or len(value) != width:
        raise RuntimeError(
            f"REG_READ: mismatch addr=0x{r_addr:04X} width={r_width} value_len={len(value)} "
            f"(expected addr=0x{addr & 0xFFFF:04X} width={width})"
        )

    if verbose:
        print(f"[{ts()}] [REG_READ] ok addr=0x{r_addr:04X} width={r_width} value:\n{hexdump(value)}")

    return value

def reg_write(ser: serial.Serial, addr: int, width: int, value: bytes, timeout_s=1.0, inter_gap_s=0.0, verbose=False):
    if value is None:
        value = b""
    if len(value) != width:
        raise RuntimeError(f"REG_WRITE: value length {len(value)} != width {width}")

    payload = struct.pack("<HB", addr & 0xFFFF, width & 0xFF) + value
    pkt = make_packet(CMD_REG_WRITE, payload)

    drain(ser, "PRE_TX_DRAIN")
    tx(ser, pkt, verbose=verbose, label="REG_WRITE_TX")

    resp = read_packet(ser, timeout_s=timeout_s, inter_gap_s=inter_gap_s, verbose=verbose)
    if resp is None:
        raise RuntimeError(f"REG_WRITE: no response (timeout={timeout_s}s)")
    if resp[0] == "bad_checksum":
        raise RuntimeError(f"REG_WRITE: checksum error: {resp}")

    _, cmd, dlen, data, _chk = resp
    if dlen != 0:
        raise RuntimeError(f"REG_WRITE: unexpected len={dlen}, data={data.hex()}")
    if cmd == CMD_REQUEST_SUCCESS:
        return True
    if cmd == CMD_REQUEST_FAILED:
        return False

    raise RuntimeError(f"REG_WRITE: unexpected cmd=0x{cmd:02X}, len={dlen}, data={data.hex()}")

def listen_dump(ser: serial.Serial, seconds: float):
    print(f"[{ts()}] [LISTEN] dumping raw bytes for {seconds:.1f}s ...")
    t0 = time.time()
    buf = b""
    while (time.time() - t0) < seconds:
        b = ser.read(1)
        if b:
            buf += b
            if len(buf) >= 256:
                print(f"[{ts()}] [LISTEN] got 256 bytes chunk:\n{hexdump(buf)}")
                buf = b""
        else:
            time.sleep(0.001)
    if buf:
        print(f"[{ts()}] [LISTEN] remaining {len(buf)} bytes:\n{hexdump(buf)}")
    print(f"[{ts()}] [LISTEN] done.")

def main():
    p = argparse.ArgumentParser()
    p.add_argument("port", help="e.g. /dev/ttyUSB0")
    p.add_argument("baud", type=int, help="e.g. 115200")
    p.add_argument("serial_u64_hex", help="e.g. 0x1122334455667788")
    p.add_argument("--timeout", type=float, default=1.0, help="packet timeout seconds")
    p.add_argument("--inter_gap", type=float, default=0.0, help="sleep seconds after each read chunk")
    p.add_argument("--retries", type=int, default=2, help="node_select retries")
    p.add_argument("--verbose", action="store_true", help="print verbose diagnostics")
    p.add_argument("--listen", type=float, default=0.0, help="just listen raw bytes for N seconds then exit")
    p.add_argument(
        "--read",
        action="append",
        default=[],
        metavar="ADDR[:WIDTH]",
        help="read register (e.g. 0x0100 or 0x0160:1). can repeat",
    )
    p.add_argument(
        "--write",
        action="append",
        default=[],
        metavar="ADDR[:WIDTH]=VALUE",
        help="write register (e.g. 0x0160:1=1). can repeat",
    )
    args = p.parse_args()

    sn = int(args.serial_u64_hex, 16)

    ser = serial.Serial(
        port=args.port,
        baudrate=args.baud,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=0.05,
    )

    ser.reset_input_buffer()
    ser.reset_output_buffer()

    print(f"[{ts()}] Opened {args.port} @ {args.baud} 8N1 (timeout={ser.timeout}s)")

    if args.listen > 0:
        listen_dump(ser, args.listen)
        ser.close()
        return

    if not args.read and not args.write:
        args.read = ["0x0100", "0x0120", "0x0140", "0x0160"]

    for spec in args.write:
        try:
            addr, width, value_int, value = parse_write_spec(spec)
        except ValueError as e:
            print(f"[{ts()}] WRITE parse error: {e}")
            ser.close()
            sys.exit(2)

        ok = node_select(
            ser,
            sn,
            timeout_s=args.timeout,
            inter_gap_s=args.inter_gap,
            verbose=args.verbose,
            retries=args.retries,
        )
        if not ok:
            print(f"[{ts()}] Node select failed before WRITE 0x{addr:04X}. (serial number/baud/배선/전원 확인)")
            ser.close()
            sys.exit(2)

        success = reg_write(
            ser,
            addr,
            width,
            value,
            timeout_s=args.timeout,
            inter_gap_s=args.inter_gap,
            verbose=args.verbose,
        )
        print(f"[{ts()}] REG 0x{addr:04X} WRITE = {value_int} ({'OK' if success else 'FAIL'})")

    for spec in args.read:
        try:
            addr, width = parse_addr_width(spec)
        except ValueError as e:
            print(f"[{ts()}] READ parse error: {e}")
            ser.close()
            sys.exit(2)

        ok = node_select(
            ser,
            sn,
            timeout_s=args.timeout,
            inter_gap_s=args.inter_gap,
            verbose=args.verbose,
            retries=args.retries,
        )
        if not ok:
            print(f"[{ts()}] Node select failed before REG 0x{addr:04X}. (serial number/baud/배선/전원 확인)")
            ser.close()
            sys.exit(2)

        data = reg_read(ser, addr, width, timeout_s=args.timeout, inter_gap_s=args.inter_gap, verbose=args.verbose)
        if addr == 0x0100 and width == 2:
            v_mv = struct.unpack("<h", data)[0]
            print(f"[{ts()}] REG 0x0100 AV1_VOLTAGE_mV = {v_mv} mV")
        elif addr == 0x0120 and width == 2:
            i_ua = struct.unpack("<H", data)[0]
            print(f"[{ts()}] REG 0x0120 AI1_CURRENT_uA = {i_ua} uA")
        elif addr == 0x0140 and width == 2:
            rh_tenths = struct.unpack("<H", data)[0]
            print(f"[{ts()}] REG 0x0140 HUMIDITY_%RH = {rh_tenths / 10.0:.1f}")
        elif addr == 0x0160 and width == 1:
            fan_on = struct.unpack("<B", data)[0]
            print(f"[{ts()}] REG 0x0160 FAN_POWER_ENABLE = {fan_on}")
        else:
            print(f"[{ts()}] REG 0x{addr:04X} READ ({width}B) = {data.hex()}")

    ser.close()

if __name__ == "__main__":
    main()
