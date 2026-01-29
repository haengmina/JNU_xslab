# Register
이 문서는 **센서 보드 기반의 레지스터 접근(REG_READ/REG_WRITE)**을 위한 설명서입니다.
현재 펌웨어에 **구현되어 실제로 동작하는 범위**만 포함합니다.

## 공통 규칙
- Packet Format은 `docs/01_protocol.md` 규격을 따릅니다.
- **한 Transaction에 하나의 Request/Response만 허용**됩니다.
- **각 REG_READ/REG_WRITE 전에 NODE_SELECT가 필요**합니다.
- Endianness: **Little-endian** (예: int16, uint16)
- REG_READ: `addr(2) + width(1) + reserved(1)`  
- REG_WRITE: `addr(2) + width(1) + value(width)`

## 레지스터 목록 (현재 구현됨)
| 주소 | 이름 | 폭 | 타입 | 접근 | 단위 | 설명 |
|--------|----------------|---|--------|---|----|----------------------------------|
| 0x0100 | AV1_VOLTAGE_mV | 2 | int16  | R | mV | 센서 보드 아날로그 입력 AV1 전압 (AM1011A) |
| 0x0120 | AI1_CURRENT_uA | 2 | uint16 | R | uA | 센서 보드 아날로그 입력 AI1 전류 |
| 0x0140 | HUMIDITY_%RH   | 2 | uint16 | R | 0.1%RH | AM1011A 습도 (AV1 기반 변환값) |
| 0x0160 | FAN_POWER_ENABLE | 1 | uint8 | R/W | bool | +12V 전원 출력 상태 (팬 전원 ON/OFF) |

## 비고
- AM1011A 습도 변환식 : **%RH = (mV / 1000) / 0.03 = mV / 30**
- 0x0140은 **0.1%RH 단위(uint16)** 로 반환됩니다. (예: 523 → 52.3%RH)
- REG_WRITE는 **0x0160에 한해 구현**되어 있습니다.
- 0x0160은 **팬 전원(+12V) 출력 상태**만 나타냅니다. (0: OFF, 1: ON)
- 0x0160에 WRITE 시: 0 → OFF, 1 → ON (그 외 값은 실패)

## 테스트 명령 (reg_test.py)
기본 실행 시 0x0100/0x0120/0x0140/0x0160을 순서대로 READ합니다.
```
python extra/script/reg_test.py /dev/ttyUSB0 19200 0x4653500d004c003c
```

### 원하는 주소만 READ
```
python extra/script/reg_test.py /dev/ttyUSB0 19200 0x4653500d004c003c --read 0x0100 --read 0x0160
```

### WRITE (팬 전원 ON/OFF)
```
python extra/script/reg_test.py /dev/ttyUSB0 19200 0x4653500d004c003c --write 0x0160=1
python extra/script/reg_test.py /dev/ttyUSB0 19200 0x4653500d004c003c --write 0x0160=0
```

### READ/WRITE 혼합
```
python extra/script/reg_test.py /dev/ttyUSB0 19200 0x4653500d004c003c --write 0x0160=1 --read 0x0160
```
