# STM32G0 HAL
[TOC]

## Overview

STM32G0xx 용 Library입니다

## How To Use

### Clone

대상 Porject의 Root에 이 Project를 Clone하거나, subrepo 등을 통해 추가합니다

### CMakeLists 수정

#### library link 설정

```cmake
... (중략)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/stm32g0-hal)

... (중략)
```

`이후,` 다음과 `target_link_libraries()`를 통해 `stm32g0-hal`를 Target에 연결해주세요

#### MCU Define

Target Project에서, 다음과 같이 STM32F1 MCU를 위한 필수 Define을 추가합니다

```cmake
... (중략)
set(CPU_OPTION -mcpu=cortex-m0plus -mfloat-abi=soft)
add_compile_options(${CPU_OPTION})
add_link_options(${CPU_OPTION})

set(compile_options
    #add an option to all of language
    $<$<BOOL:${SIZE_OPTIMIZE_ENABLE}>:-DSIZE_OPTIMIZE_ENABLE>
    -Wall
    -ftree-vectorize
    -fopt-info-vec-all=auto_vectorize_info.txt
    -DSTM32G0C1xx

... (중략)
```

> **include/cmsis/stm32f1xx.h** 참조

#### Link Script 지정

Flash 및 RAM 영역을 정의하는 Link Script를 추가해주세요

```cmake
... (중략)
set(link_options
    -TSTM32G0C1KETX_FLASH.ld
)
... (중략)
```

> **ldscript/** 참조
