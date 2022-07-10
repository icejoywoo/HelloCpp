find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS})

find_package(folly CONFIG REQUIRED)

set(FOLLY ${FOLLY_LIBRARIES})
set(FOLLY_BENCHMARK Folly::follybenchmark)

find_package(gflags COMPONENTS shared)
find_package(glog REQUIRED)

# 编译 jni 动态库
set(JNI_SOURCE_FILES java_demo_jni.cpp)
# link to velox dynamic link
add_library(jnidemo SHARED ${JNI_SOURCE_FILES})
target_link_libraries(jnidemo PRIVATE
        ${FOLLY}
        gflags_shared
        glog::glog
        velox_vector
        velox_exec
        velox_exec_test_util
        velox_vector_test_lib)