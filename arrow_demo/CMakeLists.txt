cmake_minimum_required(VERSION 3.20)

project(velox_test_project C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    message(STATUS "Using ccache: ${CCACHE_FOUND}")
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_FOUND})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_FOUND})
    # ARROW-3985: let ccache preserve C++ comments, because some of them may be
    # meaningful to the compiler
    set(ENV{CCACHE_COMMENTS} "1")
endif(CCACHE_FOUND)

# install arrow to deps
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/deps")
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps")
    message(STATUS "install arrow to `${CMAKE_CURRENT_SOURCE_DIR}/deps` ...")
    execute_process(
            COMMAND
            bash -c
            "( bash ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/install_arrow.sh)"
            RESULT_VARIABLE COMMAND_STATUS)
endif()

set(DEPS_DIR ${CMAKE_SOURCE_DIR}/deps)

if(COMMAND_STATUS EQUAL "1")
    message(FATAL_ERROR "Unable to install arrow to `${CMAKE_CURRENT_SOURCE_DIR}/deps`, "
            "please run `bash ${CMAKE_CURRENT_SOURCE_DIR}/third_party/install_velox.sh` to fix it.")
endif()

# add deps to CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH ${DEPS_DIR})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

include_directories(${DEPS_DIR}/include)

#find_package(lz4)
find_package(Arrow REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC -Wall")

add_subdirectory(thirdparty)

add_executable(arrow_udf_example arrow_udf_example.cpp)
target_link_libraries(arrow_udf_example PRIVATE arrow_static simdjson::simdjson)
# arrow_dataset_static
