#/bin/bash

cd $(dirname $0)

if [ -z "$BUILD_OUTPUT_DIR" ]; then
	BUILD_OUTPUT_DIR=build
fi

GENERATOR_OPTION=""
if ! [ -f "${BUILD_OUTPUT_DIR}/CMakeCache.txt" ]; then
	GENERATOR_OPTION="-G Ninja"
fi

cmake -S . -B ${BUILD_OUTPUT_DIR} ${GENERATOR_OPTION} $@
cmake --build ${BUILD_OUTPUT_DIR} -j$(nproc)

