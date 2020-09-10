# getB-SIMD
Playground for using SIMD for the magnetic field cache interpolation

# Clone

Assuming you want also the google bench mark code

``git clone --recursive https://github.com/AnChristos/getB-SIMD.git``

``cd getB-SIMD/``

``source buildGoogleBenchmark.sh``

otherwise 

``git clone https://github.com/AnChristos/getB-SIMD.git``

# Build code with CMake 

``mkdir build; cd build``

``cmake ../getB-SIMD``

``cmake``


# Example 

``./get_Bbench --benchmark_report_aggregates_only=true --benchmark_repetitions=20``

