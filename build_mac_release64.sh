# Build AWS SDK
# $BITNESS=64

cd src
git clone --recurse-submodules -b "1.9.79" "https://github.com/aws/aws-sdk-cpp.git"
cd aws-sdk-cpp
mkdir install
mkdir build
cd build
cmake ../ -DCMAKE_INSTALL_PREFIX="../install" -DCMAKE_BUILD_TYPE="Release" -DBUILD_ONLY="core;sts;timestream-query" -DCUSTOM_MEMORY_MANAGEMENT="OFF" -DENABLE_TESTING="OFF" -DBUILD_SHARED_LIBS="OFF" -DCPP_STANDARD="17"
make -j 4
make install
cd ../../../

PREFIX_PATH=$(pwd)
mkdir cmake-build64
cd cmake-build64
cmake ../src -DCMAKE_BUILD_TYPE="Release" -DBUILD_WITH_TESTS="ON" -DCODE_COVERAGE="OFF" -DBUILD_SHARED_LIBS="OFF" -DCMAKE_PREFIX_PATH="../src/aws-sdk-cpp/install"
make -j 4
cd ..
