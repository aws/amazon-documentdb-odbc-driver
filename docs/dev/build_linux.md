# Linux - Building the Amazon Timestream ODBC Driver

## Dependencies

The Unix shell can be used to install all the dependencies for Linux.

### Ubuntu 64-bit
*Note:* The Ubuntu 64-bit driver can be built as part of GitHub actions using the Ubuntu 20.04 hosted runner.

```sh
sudo apt update
sudo apt install git-all libcurl4-openssl-dev libssl-dev uuid-dev zlib1g-dev libpulse-dev gcc gcc-multilib  g++ g++-multilib cmake linux-headers-$(uname -r) build-essential unixodbc-dev
```
### Ubuntu 32-bit
*Note:* The Ubuntu 32-bit driver can be built as part of GitHub actions using the Ubuntu 20.04 hosted runner.

```sh
sudo dpkg --add-architecture i386
sudo apt update 
sudo apt install git-all:i386 unixodbc-dev:i386 odbcinst1debian2:i386 libodbc1:i386 libcurl4-openssl-dev:i386 libssl-dev:i386 uuid-dev:i386 cpp:i386 cpp-9:i386 gcc:i386 g++:i386 zlib1g-dev:i386 linux-headers-$(uname -r) gcc-multilib:i386 g++-multilib:i386 cmake g++-9:i386 gcc-9:i386 gcc-9-multilib:i386 g++-9-multilib:i386 binutils:i386 make:i386
```

### Amazon Linux 2 64-bit
*Note:* The Amazon Linux 2 driver cannot be built as part of GitHub Actions as there is no supported hosted runner.

```sh
# Install dependencies
sudo yum update
sudo yum install git curl-devel openssl-devel uuid-devel zlib-devel pulseaudio-libs-devel kernel-devel gcc gcc-c++ unixODBC-devel rpm-build

# Download and build CMake 3
wget https://cmake.org/files/v3.18/cmake-3.18.0.tar.gz
tar -xvzf cmake-3.18.0.tar.gz
cd cmake-3.18.0
./bootstrap
make -j4
sudo make install
cd ..
```

## Building the Amazon Timestream ODBC Driver for Amazon Linux 2 32-bit
*Note:* The Amazon Linux 32-bit driver cannot be built as part of GitHub Actions as there is no supported hosted runner. In order to build the 32-bit driver, a Centos 7 machine environment needs to be used. The following instructions are for using a CentOS docker image.

1. Install Docker
    * Install Docker and pull a [CentOS 7](https://hub.docker.com/_/centos) image.
    * Run the centos:centos7 image.
    * Start a terminal instance.
2. Install wget and unixODBC-devel
```
yum install wget -y
wget http://mirror.centos.org/centos/7/os/x86_64/Packages/unixODBC-devel-2.3.1-14.el7.i686.rpm
yum install unixODBC-devel-2.3.1-14.el7.i686.rpm -y
yum -y groupinstall "Compatibility libraries"
yum -y install libcurl-devel.i686 zlib-devel.i686 openssl-devel.i686 libatomic.i686
yum install libgcc*i686 libstdc++*i686 glibc*i686 gcc gcc-c++
```
3. Install Boost for regular expression support since GCC 4.8 doesn't implement it
```
yum install boost-devel.i686
```
4. Install and update CMake, rpm-build and make
```
wget https://github.com/Kitware/CMake/releases/download/v3.21.2/cmake-3.21.2-linux-x86_64.sh
sh cmake-3.21.2-linux-x86_64.sh
ln -s /cmake-3.21.2-linux-x86_64/bin/cmake /usr/bin/cmake
ln -s /cmake-3.21.2-linux-x86_64/bin/cpack /usr/bin/cpack
ln -s /usr/include/c++/4.8.5/i686-redhat-linux /usr/include/i386-linux-gnu
yum -y install rpm-build make
```
5. Install Git and clone the repository
```
yum install git
git clone <HTTP URL to GitHub .git file>
```
6. Build the driver
```
cd timestream-odbc
sh build_linux_release32_rpm.sh
```
*Note:* There is an error in the AWS SDK in [AWSError.h](https://github.com/aws/aws-sdk-cpp/blob/4804f46df31b7539021237c62d79c54d6429a0c4/aws-cpp-sdk-core/include/aws/core/client/AWSError.h). [Line 95](https://github.com/aws/aws-sdk-cpp/blob/4804f46df31b7539021237c62d79c54d6429a0c4/aws-cpp-sdk-core/include/aws/core/client/AWSError.h#L95) will trigger a warning since "other" is unused which will break the build. To resolve this, use a text editor, such as `vi`, to replace `AWSError& operator=(AWSError<ERROR_TYPE>&& other) = default;` with `AWSError& operator=(AWSError<ERROR_TYPE>&& ) = default; ` 

7. Proceed with the build and run the packaging
```
sh build_linux_release32_rpm.sh
cd cmake-build32
cmake ../src
make
cpack .
```
8. From the host machine, copy the installer from the Docker image
```
docker cp [container]://timestream-odbc/cmake-build32/AmazonTimestreamODBC_[version]_i386.rpm  [local path]
```

## Building the Amazon Timestream ODBC Driver for Ubuntu 64-bit, Ubuntu 32-bit or Amazon Linux 2 64-bit

From the Unix shell, navigate to the project source directory, run one of the following:
* `./build_linux_debug64.sh`
* `./build_linux_release32_deb.sh`
* `./build_linux_release64_deb.sh`
* `./build_linux_release32_rpm.sh`
* `./build_linux_release64_rpm.sh`

**Notes:**
* Ubuntu 20.04 can be used to build either the 32-bit or 64-bit deb files.
* Amazon Linux 2 should be used to build the 64-bit rpm file.
* CentOS 7 should be used to build the 32-bit rpm file.

### Output Location on Linux

Compiling on Linux will output the tests to **build/odbc/bin** and the driver to **build/odbc/lib**. There are also some additional test infrastructure files which output to the **build/odbc/lib** directory. Both 32-bit and 64-bit builds output here.

### Packaging

From the `cmake-build64` or the `cmake-build32` folder, run:

```
cmake ../src
make
cpack .
```

An installer named `AmazonTimestreamODBC-<version>_<installer_type>` will be generated in the build directory (for example `AmazonTimestreamODBC_1.0.0_x86_64.rpm` will be created in `cmake-build64`).
