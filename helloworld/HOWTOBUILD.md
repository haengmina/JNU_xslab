
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

## .deb, .rpm Package Build
```shell
(cd ./build && cpack)
```

