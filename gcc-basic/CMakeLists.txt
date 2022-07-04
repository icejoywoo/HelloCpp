cmake_minimum_required(VERSION 3.20)

project(hello)

# add googletest and googlemock
# https://crascit.com/2015/07/25/cmake-gtest/
# Download and unpack googletest at configure time
configure_file(googletest_CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download"
)
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download"
)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds the following targets:
# gtest, gtest_main, gmock and gmock_main
add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
                 "${CMAKE_BINARY_DIR}/googletest-build"
)

# The gtest/gmock targets carry header search path dependencies
# automatically when using CMake 2.8.11 or later. Otherwise we
# have to add them here ourselves.
if(CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}/include"
    )
endif()

# Now simply link your own targets against gtest, gmock,
# etc. as appropriate

# target_srouces https://crascit.com/2016/01/31/enhanced-source-file-handling-with-target_sources/
# 3.12 之前的版本，可以使用自定义的函数来解决
include("cmake/target_sources_local.cmake")

add_executable(hello "")

target_sources_local(hello
    PRIVATE
        hello.cpp
)

add_library(base64_shared SHARED "")
set_target_properties(base64_shared PROPERTIES OUTPUT_NAME "base64")

target_sources_local(base64_shared
    PRIVATE
        base64.cpp
        base64-impl.h
    PUBLIC
        base64.h  # poor PUBLIC example, see discussion below for why
)


add_library(base64_static STATIC "")
set_target_properties(base64_static PROPERTIES OUTPUT_NAME "base64")

target_sources_local(base64_static
    PRIVATE
        base64.cpp
    INTERFACE
        base64.h  # poor PUBLIC example, see discussion below for why
        base64-impl.h
)


add_executable(base64_main "")

target_sources_local(base64_main
    PRIVATE
        base64_main.cpp
        base64.cpp
    INTERFACE
        base64.h  # poor PUBLIC example, see discussion below for why
        base64-impl.h
)


add_executable(base64_main_shared "")
target_link_libraries(base64_main_shared base64_shared)

target_sources_local(base64_main_shared
    PRIVATE
        base64_main.cpp
    PUBLIC
        base64.h  # poor PUBLIC example, see discussion below for why
)

add_executable(base64_main_static "")
target_link_libraries(base64_main_static base64_static)

target_sources_local(base64_main_static
    PRIVATE
        base64_main.cpp
    PUBLIC
        base64.h  # poor PUBLIC example, see discussion below for why
)

## google test
# https://blog.xizhibei.me/2020/04/05/cmake-4-test-with-google-test/
# 1. enable_testing()
enable_testing()
# https://cmake.org/cmake/help/v3.21/module/GoogleTest.html
# https://stackoverflow.com/questions/50861636/using-google-tests-with-cmake-ctest-with-the-new-command-gtest-discover-tests
# https://www.kitware.com/dynamic-google-test-discovery-in-cmake-3-10/
include(GoogleTest) # gtest_add_tests / gtest_discover_tests

# 2. add_test
# add_test is cmake built-in method
#add_test(base64_test base64_test)
add_executable(base64_test base64_test.cpp)
target_link_libraries(base64_test base64_static gtest gtest_main)
# gtest_add_tests for google test
gtest_add_tests(TARGET base64_test AUTO)

#add_test(string_test string_test)
add_executable(string_test string_test.cpp)
target_link_libraries(string_test gtest gtest_main)
gtest_add_tests(TARGET string_test AUTO)