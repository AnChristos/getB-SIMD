# getB-SIMD
Playground for using SIMD for the magnetic field cache interpolation

# First clone/install

Assuming you want also the google bench mark code

``git clone --recursive https://github.com/AnChristos/getB-SIMD.git``

``cd getB-SIMD/``

``source buildGoogleBenchmark.sh``

# Run the test
Sanity test to see things working

``source build_test.sh ``

``./getB_test`` 

# Run the benchmark

``source buildBench.sh`` 

``./getB_bench``

#CMake

``mkdir build; cd build``
``cmake ../``
``cmake``
``./getB_bench``

# What is included so far

The code is compiled with O2 (as this is the flag used in the Athena project)
1. Standard scalar getB code
2. getB with a 1st attempt to vectorize by hand one loop
3. Enable auto vectorization (relevant for  gcc -O2)
