cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(getB-SIMD VERSION 1.0)

# C++ setting 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -pedantic -Werror  -O2 -g)

#google benchmark 
FIND_LIBRARY(GBENCH_LIB NAMES "benchmark"
  PATHS ${PROJECT_SOURCE_DIR}/benchmark/build/src )

FIND_PATH(GBENCH_INCL NAMES "benchmark"
  PATHS ${PROJECT_SOURCE_DIR}/benchmark/include )

find_package (Threads)

add_executable(getB_test 
  BFieldCache.cxx getB_test.cxx)

add_executable(getB_bench 
  BFieldCache.cxx getB_bench.cxx)

add_executable(getCache_bench
  BFieldCache.cxx getCache_bench.cxx)


target_link_libraries(getB_bench PRIVATE "${GBENCH_LIB}")
target_include_directories(getB_bench PRIVATE "${GBENCH_INCL}")
target_link_libraries (getB_bench  PRIVATE "${CMAKE_THREAD_LIBS_INIT}")

target_link_libraries(getCache_bench PRIVATE "${GBENCH_LIB}")
target_include_directories(getCache_bench PRIVATE "${GBENCH_INCL}")
target_link_libraries (getCache_bench  PRIVATE "${CMAKE_THREAD_LIBS_INIT}")

