# Build AWS SDK
# $BITNESS=32

TOOLCHAIN="$(pwd)/src/linux_32bit_toolchain.cmake"
cd src
git clone --recurse-submodules -b "1.9.79" "https://github.com/aws/aws-sdk-cpp.git"
cd aws-sdk-cpp
mkdir install
mkdir build
cd build
cmake ../ -DCMAKE_INSTALL_PREFIX="../install" -DCMAKE_BUILD_TYPE="Release" -DBUILD_ONLY="core;sts;timestream-query" -DCUSTOM_MEMORY_MANAGEMENT="OFF" -DENABLE_TESTING="OFF" -DBUILD_SHARED_LIBS="OFF" -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}" -DCMAKE_C_STANDARD="99" -DCPP_STANDARD="11"
make -j 4
make install
cd ../../../

mkdir cmake-build32
cd cmake-build32
cmake ../src -DCMAKE_BUILD_TYPE="Release" -DBUILD_WITH_TESTS="OFF" -DCODE_COVERAGE="OFF" -DBUILD_SHARED_LIBS="OFF" -DINSTALLER_TYPE="RPM" -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}" -DCMAKE_PREFIX_PATH="../src/aws-sdk-cpp/install" -DCMAKE_C_STANDARD="99" -DCPP_STANDARD="11"
make -j 4
cd ..
