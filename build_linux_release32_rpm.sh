# Build AWS SDK
# $BITNESS=32

pushd /usr/lib/i386-linux-gnu/
ln -s libssl.so.1.1  libssl.so 
ln -s libcrypto.so.1.1 libcrypto.so
popd
BASEDIR="$(pwd)"
TOOLCHAIN="${BASEDIR}/src/linux_32bit_toolchain.cmake"
cd src
git clone --recurse-submodules -b "1.8.186" "https://github.com/aws/aws-sdk-cpp.git"
cd aws-sdk-cpp
mkdir install
mkdir build
cd build
cmake ../ -DCMAKE_BUILD_TYPE="Release" -DBUILD_ONLY="core;sts;timestream-query" -DCUSTOM_MEMORY_MANAGEMENT="OFF" -DENABLE_TESTING="OFF" -DBUILD_SHARED_LIBS="OFF" -DCMAKE_CXX_FLAGS="-m32" -DCMAKE_C_FLAGS="-m32" -DCMAKE_SHARED_LINKER_FLAGS="-m32"
# Rerun to set the install prefix (https://github.com/aws/aws-sdk-cpp/issues/1156)
cmake ../ -DCMAKE_INSTALL_PREFIX="../install" -DCMAKE_BUILD_TYPE="Release" -DBUILD_ONLY="core;sts;timestream-query" -DCUSTOM_MEMORY_MANAGEMENT="OFF" -DENABLE_TESTING="OFF" -DBUILD_SHARED_LIBS="OFF" -DCMAKE_CXX_FLAGS="-m32" -DCMAKE_C_FLAGS="-m32" -DCMAKE_SHARED_LINKER_FLAGS="-m32"
make -j 4
make install
cd ../../../

PREFIX_PATH=$(pwd)
mkdir cmake-build32
cd cmake-build32
cmake ../src -DCMAKE_BUILD_TYPE="Release" -DBUILD_WITH_TESTS="ON" -DCODE_COVERAGE="OFF" -DBUILD_SHARED_LIBS="OFF" -DINSTALLER_TYPE="RPM" -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}"
make -j 4
cd ..
