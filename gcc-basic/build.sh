#!/bin/bash

# build simple `hello world` program
# 只依赖 stl
g++ hello.cpp -o hello
# compiler: 
# 1. g++ for c++
# 2. gcc for c

# -c: Only run preprocess, compile, and assemble steps
# -I <dir>: Add directory to the end of the list of include search paths
# -o <file>: Write output to <file>
g++ -c base64.cpp -o base64.o -I./
g++ -c base64_main.cpp -o base64_main.o -I./
# build dynamic library
g++ -shared base64.o -o libbase64.so -install_name @rpath/libbase64.so
# build static library
ar rvs libbase64.a base64.o

# executable with .o files
g++ base64.o base64_main.o -o base64_main
# executable with static library
g++ base64_main.o libbase64.a -o base64_main_static
# executable with dynamic library
g++ base64_main.o -o base64_main_shared -lbase64 -L./

# 如果没有用到 basey64.h 的函数，这个时候其实并不需要 base64.o，是可以编译过的
# g++ base64_main.o -o base64_main
