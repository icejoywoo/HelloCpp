cmake_minimum_required(VERSION 3.5)

project(hello)

add_executable(hello hello.cpp)

add_library(base64_shared SHARED base64.cpp)
set_target_properties(base64_shared PROPERTIES OUTPUT_NAME "base64")

add_library(base64_static STATIC base64.cpp)
set_target_properties(base64_static PROPERTIES OUTPUT_NAME "base64")

add_executable(base64_main base64_main.cpp base64.cpp)

add_executable(base64_main_shared base64_main.cpp)
target_link_libraries(base64_main_shared base64_shared)

add_executable(base64_main_static base64_main.cpp)
target_link_libraries(base64_main_static base64_static)