# Build AWS SDK
# $BITNESS=64

mkdir cmake-build64
cd cmake-build64
cmake ../src -DCMAKE_BUILD_TYPE="Debug" -DBUILD_WITH_TESTS="ON" -DCODE_COVERAGE="ON" -DBUILD_SHARED_LIBS="OFF"
make ccov-all -j 4
cd ..
