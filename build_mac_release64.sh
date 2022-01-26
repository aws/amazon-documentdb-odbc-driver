mkdir cmake-build64
cd cmake-build64
cmake ../src -DCMAKE_BUILD_TYPE="Release" -DCODE_COVERAGE="OFF" -DBUILD_SHARED_LIBS="OFF" -DWITH_TESTS="ON" -DWITH_CORE="OFF" -DWITH_ODBC="ON"
make -j 4
cd ..
