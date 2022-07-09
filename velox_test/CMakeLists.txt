cmake_minimum_required(VERSION 3.20)

project(modern-cpp-demo)

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

execute_process(
  COMMAND
    bash -c
    "( source ${CMAKE_CURRENT_SOURCE_DIR}/velox/scripts/setup-helper-functions.sh && echo -n $(get_cxx_flags $ENV{CPU_TARGET}))"
  OUTPUT_VARIABLE SCRIPT_CXX_FLAGS
  RESULT_VARIABLE COMMAND_STATUS)

if(COMMAND_STATUS EQUAL "1")
  message(FATAL_ERROR "Unable to determine compiler flags!")
endif()
message("Setting CMAKE_CXX_FLAGS=${SCRIPT_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SCRIPT_CXX_FLAGS}")

add_subdirectory(velox)

set(VELOX_LIBS
        velox_temp_path
        velox_dwio_type_fbhive
        velox_function_registry
        velox_expression
        velox_duckdb_parser
        velox_type_tz
        velox_duckdb_conversion
        velox_read_benchmark_lib
        velox_connector
        velox_dwio_dwrf_proto
        velox_memory
        velox_serialization
        velox_core
        velox_vector
        velox_functions_lib
        velox_type_calculation
        velox_parse_parser
        velox_dwio_common_compression
        velox_process
        velox_dwio_dwrf_utils
        velox_file
        velox_flag_definitions
        velox_dwio_dwrf_writer
        velox_presto_serializer
        velox_parse_expression
        velox_functions_prestosql
        velox_functions_spark
        velox_hive_partition_function
        velox_codegen
        velox_duckdb_functions
        velox_arrow_bridge
        velox_dwio_dwrf_reader
        velox_expression_functions
        velox_vector_test_lib
        velox_dwio_common
        velox_aggregates_test_lib
        velox_external_date
        velox_config
        velox_e2e_filter_test_base
        velox_functions_json
        velox_vector_fuzzer
        velox_caching
        velox_type
        velox_time
        velox_exec
        velox_common_base
        velox_dwrf_test_utils
        velox_dwio_common_exception
        velox_row
        velox_encode
        velox_functions_test_lib
        velox_exec_test_util
        velox_dwio_common_encryption
        velox_buffer
        velox_functions_hyperloglog
        velox_expression_fuzzer
        velox_tpch_gen
        velox_parse_utils
        velox_presto_types
        velox_exception
        velox_dwio_dwrf_common)

include_directories(velox)
add_executable(velox_limit_demo velox_limit_demo.cc)
target_link_libraries(velox_limit_demo velox_exec velox_exec_test_util velox_vector_test_lib)
#target_link_libraries(velox_limit_demo ${VELOX_LIBS})

# target_link_libraries(velox_limit_demo PRIVATE
#         glog::glog
#         xsimd
#         gtest
#         folly
#         ${FOLLY_WITH_DEPENDENCIES}
#         fmt
#         ${FMT}
#         ${GFLAGS_LIBRARIES}
#         pthread
#         protobuf::libprotobuf
#         ${RE2}
#         ${VELOX_LIBS})