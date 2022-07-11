#!/bin/bash

# use `CMAKE_PREFIX_PATH` to find third-party libraries, like folly, fmt etc.
# source this file, then compile project using `cmake .. && cmake --build .`
CURRENT_DIR=$(cd `dirname $0`; pwd)
export CMAKE_PREFIX_PATH=${CURRENT_DIR}/deps/
