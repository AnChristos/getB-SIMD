# getB-SIMD
Playground for using SIMD for the magnetic field cache interpolation

# Google benchmark

``git clone https://github.com/google/benchmark.git``

The following script from this repo will install the benchmark under ${HOME}/.local

``source buildGoogleBenchmark.sh``

# Install  getB-SIMD

``git clone https://github.com/AnChristos/getB-SIMD.git``

# Build code with CMake 

``mkdir build; cd build``

``cmake ../getB-SIMD``

``cmake``

# Example 

``./getB_bench --benchmark_report_aggregates_only=true --benchmark_repetitions=20``

