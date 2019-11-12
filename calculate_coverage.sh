rm -rf coverage
rm -rf cmake-build-debug-coverage-clang
mkdir cmake-build-debug-coverage-clang
cd cmake-build-debug-coverage-clang
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
make -j4 hyriseCoverage
cd ..
./scripts/coverage.sh cmake-build-debug-coverage-clang
