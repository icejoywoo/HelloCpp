#!/bin/bash

set -u
set -x

CURRENT_DIR=$(cd `dirname $0`; pwd)
INSTALL_DEPS_DIR=${CURRENT_DIR}/../deps
if [ ! -d ${INSTALL_DEPS_DIR} ]; then
  mkdir ${INSTALL_DEPS_DIR}
fi
ARROW_INSTALL_PREFIX=$(cd ${INSTALL_DEPS_DIR}; pwd)

echo "arrow install path: ${ARROW_INSTALL_PREFIX}"

cd ${CURRENT_DIR}/arrow/cpp

BUILD_DIR=build-install
if [ ! -d ${BUILD_DIR} ]; then
  rm -rf ${BUILD_DIR}
fi

mkdir ${BUILD_DIR}

cd ${BUILD_DIR}

cmake .. --preset ninja-debug -DCMAKE_INSTALL_PREFIX=${ARROW_INSTALL_PREFIX} -DARROW_EXTRA_ERROR_CONTEXT=ON -DARROW_BUILD_STATIC=ON

cmake --build .

ninja install