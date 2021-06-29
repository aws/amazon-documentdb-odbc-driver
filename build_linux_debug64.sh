# Build AWS SDK
# $BITNESS=64

cd src
git clone --recurse-submodules -b "1.8.186" "https://github.com/aws/aws-sdk-cpp.git"
cd aws-sdk-cpp
mkdir install
mkdir build
cd build
cmake ../ -DCMAKE_BUILD_TYPE="Debug" -DBUILD_ONLY="core;sts;timestream-query" -DCUSTOM_MEMORY_MANAGEMENT="OFF" -DENABLE_TESTING="OFF" -DBUILD_SHARED_LIBS="OFF"
# Rerun to set the install prefix (https://github.com/aws/aws-sdk-cpp/issues/1156)
cmake ../ -DCMAKE_INSTALL_PREFIX="../install" -DCMAKE_BUILD_TYPE="Debug" -DBUILD_ONLY="core;sts;timestream-query" -DCUSTOM_MEMORY_MANAGEMENT="OFF" -DENABLE_TESTING="OFF" -DBUILD_SHARED_LIBS="OFF"
make -j 4
make install
cd ../../../

PREFIX_PATH=$(pwd)
mkdir cmake-build64
cd cmake-build64
#INSTALLER_TYPE is required but irrelevant to this.
cmake ../src -DCMAKE_BUILD_TYPE="Debug" -DBUILD_WITH_TESTS="ON" -DCODE_COVERAGE="ON" -DBUILD_SHARED_LIBS="OFF" -DINSTALLER_TYPE="RPM"
make ccov-all -j 4
cd ..
