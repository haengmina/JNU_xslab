# JLink RTT

[TOC]

## Description

JTAG/SWD 등의 Debug Interface를 기반으로 고속 stdin/stdout 을 구현합니다

## How To Use

### Clone

대상 Porject의 Root에 이 Project를 Clone하거나, subrepo 등을 통해 추가합니다

### CMakeLists 수정

#### library link 설정

```cmake
... (중략)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/jlink-rtt)

... (중략)
```

`이후,` 다음과 `target_link_libraries()`를 통해 `segger_rtt`를 Target에 연결해주세요
link 후에는 jlink-rtt가 기본 stdin/stdout 으로 설정됩니다
