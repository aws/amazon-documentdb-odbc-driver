# Build AWS SDK
# $BITNESS=64

mkdir cmake-build64
cd cmake-build64
cmake ../src -DCMAKE_BUILD_TYPE="Debug" -DCODE_COVERAGE="ON" -DBUILD_SHARED_LIBS="OFF" -DWITH_TESTS="OFF" -DWITH_CORE="OFF" -DWITH_ODBC="ON"
make ccov-all -j 4
cd ..
