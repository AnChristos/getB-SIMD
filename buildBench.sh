g++ BFieldCache.cxx getB_bench.cxx -std=c++17  -O2 -isystem benchmark/include -Lbenchmark/build/src -lbenchmark -lpthread -o getBbench
#./getBbench --benchmark_report_aggregates_only=true --benchmark_repetitions=20
