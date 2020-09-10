cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(getB-SIMD VERSION 1.0)

# C++ setting 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)
add_compile_options(-Wall -Wextra -pedantic -Werror  -O2 -g)

find_package(benchmark)
find_package (Threads)

add_executable(getB_test 
  BFieldCache.cxx getB_test.cxx)
add_executable(getB_bench 
  BFieldCache.cxx getB_bench.cxx)
add_executable(getCache_bench
  BFieldCache.cxx getCache_bench.cxx)


target_link_libraries(getB_bench benchmark::benchmark)
target_link_libraries(getCache_bench benchmark::benchmark)
target_link_libraries (getCache_bench  benchmark::benchmark)

