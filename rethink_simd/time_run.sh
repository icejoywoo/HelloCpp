#!/bin/bash

set -x
set -u
set -e

BUILD_DIR=_build

time (
    rm -rf ${BUILD_DIR}
    mkdir ${BUILD_DIR}
    cd ${BUILD_DIR}
    cmake .. -G Ninja
    cmake --build .
    ctest
)

#rm -rf build
