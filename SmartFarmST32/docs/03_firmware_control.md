# Firmware 제어 명령
이 문서에서는 **노드의 Firmware를 제어하는 명령과 흐름을** 다룹니다

@important **[Protocol - Packet Format](docs/01_protocol.md#packet-format) 규격을 준수해야합니다**

---
## Version Read
**버전 정보를** 읽습니다

### Requst
- `Command:` **0xA0 (FIRMWARE_VERSION_READ)**
- `Data Length:` 0
- `Data:`

### Response
- `Command:` **0xA1 (FIRMWARE_VERSION_RESPONSE)**
- `Data Length:` 3
- `Data:` 다음 규격을 따릅니다
```
/*!
 * |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                 Major Version                 | [0]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                 Minor Version                 | [1]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                 Patch Version                 | [2]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 */
```

---
## Firmware File Data 규격
[Intel HEX File](https://en.wikipedia.org/wiki/Intel_HEX)을 사용합니다
@note
바이너리 데이터를 **ASCII 형태로 기록해** 둔 것을 의미합니다
- **EEPROM 및 MCU의 Firmware를 저장하기 위한 목적으로** 많이 사용됩니다

```
  -- 시작 코드: 레코드의 시작을 나타냅니다
  |    -- 데이터 길이: 데이터 부분의 길이를 나타냅니다
  |    |     -- 데이터 주소: 데이터가 기록될 위치를 나타냅니다
  |    |     |     -- 레코드 타입: 00: Data, 01: EOF(End of File)
  |    |     |     |                   -- Data: Flash 등에 기록될 데이터입니다
  |    |     |     |                   |                   -- Checksum: 데이터 무결성 검사를 위해 사용됩니다
  |    |     |     |                   |                   |
| 1 |  2 |   3  |  4 |                 5                |  6 |
  :   10   4000   00   38150020096400000564000005640000   04   -- 각 줄을 "레코드"라고 지칭합니다
  :   10   4010   00   00000000000000000000000000000000   A0
  :   10   4020   00   00000000000000000000000005640000   27
... (중략)
  :   08   CA90   00   0000000000000000                   9E
  :   04   0000   03   00004000                           B9
  :   00   0000   01                                      FF
```

---
## Update Flow
`통합 제어기` 를 통해 **새로운 Firmware를 업데이트할 수 있는 기능을** 제공합니다

전체 과정은 **다음과 같이 `2 단계로` 진행됩니다**
```plantuml
participant "통합 제어기" as controller order 0
participant "센서/제어 노드 1" as node1 order 1

group 1. Firmware Upload 과정
group 전체 Firmware 데이터를 복사할 때까지 반복
controller --> node1: **지정된 Serial Number를 가지는 노드** 선택\n( NODE_SELECT_REQUEST )
activate node1
activate node1
controller <-- node1: **노드 선택에 대한 Response** 전송\n( NODE_SELECT_RESPONSE )
deactivate node1
activate controller
controller --> node1: \
**Firmware Record 전송**\n( FIRMWARE_REQUEST )\n\
( **2초 이내에 Response가 오지 않은 경우, 재전송** )
activate node1
node1 --> node1: 수신된 **Firmware Record 유효성 검사 및 저장**
controller <-- node1: \
**정상적으로 처리된 경우:** REQUEST_SUCCESS 전송\n\
**유효성 검사에 실패한 경우:** REQUEST_FAILED 전송\n\
( **REQUEST_FAILED이 수신된 경우,** 처음부터 다시 전송 필요 )
deactivate node1
deactivate node1
deactivate controller
end
node1 --> node1: Firmware **Update Request Flag 설정**
end

node1 --> node1: Watchdog에 의해 **System Reset**

group 2. Firmware Update 과정
group Boot Loader
node1 --> node1: Firmware **Update Request Flag 검사**
group 전체 영역을 복사할 때까지 반복
activate node1
activate node1
node1 --> node1: 64bit 단위로 **다운로드된 Firmware를 Main Firmware 영역에 복사**
deactivate node1
end
node1 --> node1: Firmware **Update Request Flag 초기화**
node1 --> node1: NVIC에 의해 **System Reset**
deactivate node1
node1 --> node1: **새로운 Firmware** 시작
activate node1
end
end

node1 --> node1: **Isolated RS485 (RJ45) 초기화**
node1 --> node1: **통합 제어기로 부터 명령 수신 기능 활성화**
```

---
## Firmware 데이터 전송
대상 HEX 파일의 **Record를 한개씩 전송합니다**
> `따라서,` **각 Record 전송 마다 Node Select 과정이** 필요합니다

`이때,` ASCII 형태의 Record에서 `시작 코드를 제외한` **데이터 길이부터 Checksum 부분까지를** `Binary로 디코딩 후` 전송합니다

- `Command:` **0xA2 (FIRMWARE_REQUEST)**
- `Data Length:` Firmware Record Length + 2 (Length 및 Checksum Byte 포함)
- `Data:` 다음 규격을 따릅니다
```
/*!
 * |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                  Data Length                  | [0]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                  Address MSB                  | [1]
 * |                                               |
 * |                  Address LSB                  | [2]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                     Type                      | [3]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                                               | [4]
 * ~                     Data                      ~
 * |                                               | [N - 1]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                   CheckSum                    | [N]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 */
```

@note 이 명령에 대한 **응답은 2초 이내에 수신되어야하며,** 수신되지 않은 경우 **Checksum 오류가 발생한 것으로 간주하여 재전송이 필요합니다**

### Response
- `Command:` **0x24 (REQUEST_SUCCESS)** 또는 **0x23 (REQUEST_FAILED)**
- `Data Length:` 0
- `Data:`

@important **REQUEST_FAILED를** 수신한 경우, **처음부터 다시 전송해야합니다**

