cd benchmark/
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_INSTALL_PREFIX=${HOME}/.local ../ 
cmake --build --config Release --target install 
cd ../
