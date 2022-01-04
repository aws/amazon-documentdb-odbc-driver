# Build AWS SDK
# $BITNESS=64

mkdir cmake-build64
cd cmake-build64
cmake ../src -DCMAKE_BUILD_TYPE="Release" -DBUILD_WITH_TESTS="ON" -DCODE_COVERAGE="OFF" -DBUILD_SHARED_LIBS="OFF"
make -j 4
cd ..
