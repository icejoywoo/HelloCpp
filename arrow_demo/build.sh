#!/bin/bash

set -x
set -u
set -e

CMAKE_ARGS="-DOPENSSL_ROOT_DIR=$(brew --prefix openssl@1.1)"

time (
	rm -rf _build
	mkdir _build
	cd _build
	cmake .. -G Ninja ${CMAKE_ARGS}
	cmake --build .
	ctest
)
