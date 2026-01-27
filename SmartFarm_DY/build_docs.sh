#/bin/bash

cd $(dirname $0)

if [ -z "$BUILD_OUTPUT_DIR" ]; then
	BUILD_OUTPUT_DIR=build
fi

GENERATOR_OPTION=""
if ! [ -f "${BUILD_OUTPUT_DIR}/CMakeCache.txt" ]; then
	GENERATOR_OPTION="-G Ninja"
fi

for arg in "$@"; do
	DOXYGEN_INPUT_OVERRIDE+="$(realpath $arg) "
done

cmake -S . -B ${BUILD_OUTPUT_DIR} ${GENERATOR_OPTION} -DSKIP_PDF=${SKIP_PDF} -DDOXYGEN_INPUT_OVERRIDE="${DOXYGEN_INPUT_OVERRIDE}"
TARGET_NAME=$(cat ${BUILD_OUTPUT_DIR}/CMakeCache.txt | awk -v word="CMAKE_PROJECT_NAME" '$0 ~ word { match($0, "=.*"); print substr($0, RSTART+1, RLENGTH) }')

cmake --build ${BUILD_OUTPUT_DIR} --target doxygen_${TARGET_NAME}

