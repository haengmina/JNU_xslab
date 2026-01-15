
# How to Build

[TOC]

## Only Docs Build

```shell
./build_docs.sh
```

## Release With Debug Info Mode Build

```shell
./build.sh
```

## Release Mode Build

```shell
./build.sh -DCMAKE_BUILD_TYPE=Release
```

## Debug Mode Build

```shell
./build.sh -DCMAKE_BUILD_TYPE=Debug
```

## Release With Debug Info Mode Build with Toolchain

```shell
./build.sh -DCMAKE_TOOLCHAIN_FILE=./toolchain/${TARGET_TOOLCHAIN_FILE}
```

## Release With Debug Info Mode Build with Specific Toolchain

```shell
./build.sh -DCMAKE_TOOLCHAIN_FILE=./toolchain/${TARGET_TOOLCHAIN_FILE} -DTOOLCHAIN_ROOT=${TARGET_TOOLCHAIN_ROOT}
```

## Release With Debug Info Mode Build with Toolchain, sysroot

```shell
./build.sh -DCMAKE_TOOLCHAIN_FILE=./toolchain/${TARGET_TOOLCHAIN_FILE} -DCMAKE_SYSROOT=${SYSROOT_PATH}
```

## Debug Mode Build with Toolchain

```shell
./build.sh -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./toolchain/${TARGET_TOOLCHAIN_FILE}
```

## .deb, .rpm Package Build

```shell
(cd ./build && cpack)
```
