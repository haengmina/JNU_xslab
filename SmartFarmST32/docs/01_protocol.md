# Protocol
이 문서에서는 `통합 제어기` `<--->` `제어/센서 노드` 사이에 사용되는 **통신 프로토콜에 관해 다룹니다**

## Transaction
`통합 제어기` 가 **대상 `노드`를 선택한 시점 부터 `노드`가 응답을 완료하는 시점까지를** `Transaction` 이라고 합니다

## Flow Chart
```plantuml
participant "통합 제어기" as controller order 0
participant "센서/제어 노드 1" as node1 order 1
participant "센서/제어 노드 2" as node2 order 2

controller --> node1: **지정된 Serial Number를 가지는 노드** 선택\n( NODE_SELECT_REQUEST )
note left node1: Transcation 시작
activate node1
activate node1
controller <-- node1: **노드 선택에 대한 Response** 전송\n( NODE_SELECT_RESPONSE )
deactivate node1
activate controller
controller --> node1: 노드에 대한 **제어 Request** 전송
activate node1
node1 --> node1: 요청에 대한 처리 수행\n**종류에 따라, Response 전송까지 시간이 소비될 수** 있음
controller <-- node1: **제어 요청에 대한 Response** 전송\nCommand Type에 따라\n**Request가 정상적으로 처리되었는지 여부를** 확인 가능
note over controller, node1: **한 번의 Transaction에서 하나의 Request 및 Response만** 허용\nResponse를 전송하는 시점에서 **Transaction이 종료되며, 노드 선택 해제**
deactivate node1
deactivate node1
deactivate controller

controller --> node2: **지정된 Serial Number를 가지는 노드** 선택\n( NODE_SELECT_REQUEST )
note left node2: Transcation 시작
activate node2
activate node2
controller <-- node2: **노드 선택에 대한 Response** 전송\n( NODE_SELECT_RESPONSE )
deactivate node2
activate controller
controller --> node2: 노드에 대한 **제어 Request** 전송
activate node2
node2 --> node2: 요청에 대한 처리 수행\n**종류에 따라, Response 전송까지 시간이 소비될 수** 있음
controller <-- node2: **제어 요청에 대한 Response** 전송\nCommand Type에 따라\n**Request가 정상적으로 처리되었는지 여부를** 확인 가능
note over controller, node2: **한 번의 Transaction에서 하나의 Request 및 Response만** 허용\nResponse를 전송하는 시점에서 **Transaction이 종료되며, 노드 선택 해제**
deactivate node2
deactivate node2
deactivate controller
```


---
## 제한 사항
- **한 번에 하나의 노드와** 통신할 수 있습니다
- **노드가 선택된 상태에서, Request를 전송하지 않고 다른 노드를 선택하는 경우** 기존 노드의 선택이 취소됩니다
- **한 번의 Transaction에서 하나의 Request 및 Response만** 허용됩니다
- **Request에 대한 Response를 전송하는 시점에,** Transaction이 종료됩니다
- **Byte 수신 후 다음 Byte는 100ms 이내에** 수신되어야합니다


---
## 통신 정보
- **통신 커넥터:** RJ45
- **전기적 규격:** RS485
- **바이트 프토토콜:** UART (19200 bps, 8 bit, no parity, stop bit 1)


---
## Packet Format
**다음과 같은 Format을 가진 Packet을 사용하여** 통신합니다

**Data 영역에 용도에 맞는 데이터를 추가하여** 사용할 수 있습니다

```
/*!
 * @code
 * Packet Format
 * 
 * [0] : Start Byte (0x7E)
 * [1] : Command Type
 * [2] : Data Length
 * [3 ~ N - 1] : Data
 * [N] : XOR Checksum
 *
 * |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                  Start Byte                   | [0]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                    Command                    | [1]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                  Data Length                  | [2]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                                               | [3]
 * ~                      Data                     ~
 * |                                               | [N - 1]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                 XOR Checksum                  | [N]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * Command List
 * - NODE_SELECT_REQUEST = 0x20
 * - NODE_SELECT_RESPONSE = 0x21
 * - ERROR_INVALID_REQUEST = 0x22
 * - REQUEST_FAILED = 0x23
 * - REQUEST_SUCCESS = 0x24
 * - DI_READ = 0x30
 * - DI_THRESHOLD_WRITE = 0x31
 * - DO_READ = 0x32
 * - DO_WRITE = 0x33
 * - DI_RESPONSE = 0x40
 * - DO_RESPONSE = 0x41
 * - DI_DO_READ_ALL = 0x42
 * - DI_DO_READ_ALL_RESPONSE = 0x43
 * - DO_WRITE_ALL = 0x44
 * - ANALOG_READ = 0x50
 * - ANALOG_READ_ALL = 0x51
 * - ANALOG_RESPONSE = 0x60
 * - ANALOG_ALL_RESPONSE = 0x61
 * - SERIAL_SETUP = 0x70
 * - SERIAL_SETUP_READ = 0x71
 * - SERIAL_WRITE = 0x80
 * - SERIAL_WRITE_RESPONSE = 0x81
 * - SERIAL_SETUP_READ_RESPONSE = 0x82
 * - ACCEL_READ = 0x90
 * - ACCEL_RESPONSE = 0x91
 * - GPS_READ = 0x92
 * - GPS_RESPONSE = 0x93
 * - FIRMWARE_VERSION_READ = 0xA0
 * - FIRMWARE_VERSION_RESPONSE = 0xA1
 * - FIRMWARE_REQUEST = 0xA2
 * @code
 */
```

### Checksum
`Start Byte` 부터 `Data 영역까지의` 데이터를 XOR하여 Checksum을 생성합니다

@note `초기값은 0x00입니다`

### 공통 Request
**제어 노드 및 센서 노드에서 사용할 수 있는** 명령입니다
- `NODE_SELECT_REQUEST:` 0x20
> **통합제어기에서 Transaction을 시작하기 위해,** node 선택하는 용도로 사용됩니다

- `ACCEL_READ:` 0x90
> **3축 가속도 센서의 값을** 읽는 명령입니다

- `GPS_READ:` 0x92
> **GPS 데이터를** 읽는 명령입니다

- `FIRMWARE_VERSION_READ:` 0xA0
> **Firmware의 버전 정보를** 읽는 명령입니다

- `FIRMWARE_REQUEST:` 0xA3
> **새로운 Firmware 데이터를** 쓰는 명령입니다

### Control Interface Board 전용 Request
**제어 노드에서만 사용할 수 있는** 명령입니다
- `DI_READ:` 0x30
> **DI (Digital Input)의 상태를** 읽는 명령입니다

- `DI_THRESHOLD_WRITE:` 0x31
> **DI (Digital Input)의 Logic High로 인식하는 Threshold를** 설정하는 명령입니다

- `DO_READ:` 0x32
> **DO (Digital Output)의 상태를** 읽는 명령입니다

- `DO_WRITE:` 0x33
> **DO (Digital Output)의 상태를** 설정하는 명령입니다

- `DI_DO_READ_ALL:` 0x42
> **모든 DI (Digital Input) 및 DO (Digital Output)의 상태를** 읽는 명령입니다

- `DO_WRITE_ALL:` 0x44
> **모든 DO (Digital Output)의 상태를** 설정하는 명령입니다

### Sensor Interface Board 전용 Request
**센서 노드에서만 사용할 수 있는** 명령입니다
- `ANALOG_READ:` 0x50
> **AV (Analog Voltage) 및 AI (Analog Current)의 값을** 읽는 명령압니다

- `ANALOG_READ_ALL:` 0x51
> **모든 AV (Analog Voltage) 및 AI (Analog Current)의 값을** 읽는 명령압니다

- `SERIAL_SETUP:` 0x70
> **DB9 커넥터의 통신 설정을 변경하는** 명령입니다

- `SERIAL_SETUP_READ:` 0x71
> **DB9 커넥터의 통신 설정을 읽는** 명령입니다

- `SERIAL_WRITE:` 0x80
> **DB9 커넥터를 통해 지정된 데이터를 쓰고, 응답을 읽는** 명령입니다

### Response
- `NODE_SELECT_RESPONSE:` 0x21
> `NODE_SELECT_REQUEST`에서 **지정된 Serial Number와 현재 노드의 Serial Number가 일치하는 경우에** 전송되는 Response입니다

- `ERROR_INVALID_REQUEST:` 0x22
> **현재 구성에서 사용할 수 없는 명령을 수신한 경우** 전송되는 Response입니다

- `REQUEST_FAILED:` 0x23
> **유요하지 않은 Request Packet을 수신한 경우** 전송되는 Response입니다

- `REQUEST_SUCCESS:` 0x24
> **Request가 정상적으로 처리된 경우** 전송되는 Response입니다

- `DI_RESPONSE:` 0x40
> **`DI_READ`에 대한 Respone입니다**

- `DO_RESPONSE:` 0x41
> **`DO_READ`에 대한 Respone입니다**

- `DI_DO_READ_ALL_RESPONSE:` 0x43
> **`DI_DO_READ_ALL`에 대한 Respone입니다**

- `ANALOG_RESPONSE:` 0x60
> **`ANALOG_READ`에 대한 Respone입니다**

- `ANALOG_ALL_RESPONSE:` 0x61
> **`ANALOG_READ_ALL`에 대한 Respone입니다**

- `SERIAL_WRITE_RESPONSE:` 0x81
> **`SERIAL_WRITE`에 대한 Respone입니다**

- `SERIAL_SETUP_READ_RESPONSE:` 0x82
> **`SERIAL_SETUP_READ`에 대한 Respone입니다**

- `ACCEL_RESPONSE:` 0x91
> **`ACCEL_READ`에 대한 Respone입니다**

- `GPS_RESPONSE:` 0x93
> **`GPS_READ`에 대한 Respone입니다**

- `FIRMWARE_VERSION_RESPONSE:` 0xA1
> **`FIRMWARE_VERSION_READ`에 대한 Respone입니다**
