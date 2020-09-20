# getB-SIMD
Playground for using SIMD for the magnetic field cache interpolation


# If you have already google benchmark installed

``git clone https://github.com/AnChristos/FindMinimumIndex.git``

# Also install google benchmark 

Assuming you want also the google bench mark code

``git clone --recursive https://github.com/AnChristos/getB-SIMD.git``

``cd getB-SIMD/``

The following Will install the benchmark under /user/.local

``source buildGoogleBenchmark.sh``

edit

``cmake -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_INSTALL_PREFIX=${HOME}/.local ../ ``

to change location

# Build code with CMake 

``mkdir build; cd build``

``cmake ../getB-SIMD``

``cmake``


# Example 

``./get_Bbench --benchmark_report_aggregates_only=true --benchmark_repetitions=20``

