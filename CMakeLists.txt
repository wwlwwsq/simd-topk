cmake_minimum_required(VERSION 3.14)
project(simd_topk CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    message(FATAL_ERROR "simd-topk 面向 Linux / GCC / Clang 工具链（WSL2），请用 g++ 或 clang++ 构建")
endif()

option(VTOP_BUILD_TESTS "构建一致性测试" ON)
option(VTOP_BUILD_BENCH "构建基准" ON)
option(VTOP_SANITIZE "AddressSanitizer + UBSan" OFF)
option(VTOP_TSAN "ThreadSanitizer（与 SANITIZE 互斥）" OFF)
set(VTOP_BPOOL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../bounded-threadpool/include"
    CACHE PATH "bpool 头文件目录（bench_parallel 并行查询基准依赖）")

find_package(Threads REQUIRED)

add_library(vtop
    src/dataset.cpp
    src/distance.cpp
    src/distance_avx2.cpp
    src/topk.cpp)
target_include_directories(vtop PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(vtop PUBLIC Threads::Threads)
target_compile_options(vtop PRIVATE -Wall -Wextra -Wpedantic)

# 只有 AVX2 翻译单元带向量化标志，其余代码保持可移植
set_source_files_properties(src/distance_avx2.cpp PROPERTIES COMPILE_OPTIONS "-mavx2;-mfma")

if(VTOP_SANITIZE AND VTOP_TSAN)
    message(FATAL_ERROR "ASan 与 TSan 不能同开")
endif()

function(apply_sanitizers target)
    if(VTOP_SANITIZE)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    elseif(VTOP_TSAN)
        target_compile_options(${target} PRIVATE -fsanitize=thread -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=thread)
    endif()
endfunction()

apply_sanitizers(vtop)

if(VTOP_BUILD_TESTS)
    enable_testing()
    add_executable(test_consistency tests/test_consistency.cpp)
    target_link_libraries(test_consistency PRIVATE vtop)
    apply_sanitizers(test_consistency)
    add_test(NAME consistency COMMAND test_consistency)
endif()

if(VTOP_BUILD_BENCH)
    add_executable(bench_topk benchmarks/bench_topk.cpp)
    target_link_libraries(bench_topk PRIVATE vtop)

    if(EXISTS "${VTOP_BPOOL_INCLUDE_DIR}/bpool/thread_pool.hpp")
        add_executable(bench_parallel benchmarks/bench_parallel.cpp)
        target_link_libraries(bench_parallel PRIVATE vtop)
        target_include_directories(bench_parallel PRIVATE "${VTOP_BPOOL_INCLUDE_DIR}")
    else()
        message(STATUS "未找到 bpool 头文件（VTOP_BPOOL_INCLUDE_DIR），跳过 bench_parallel")
    endif()
endif()
