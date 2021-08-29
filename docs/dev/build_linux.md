# Linux - Building the Amazon Timestream ODBC Driver

## Linux

### Dependencies

The Unix shell can be used to install all the dependencies for Linux.

#### Ubuntu 64-bit
```sh
sudo apt update
sudo apt install libcurl4-openssl-dev libssl-dev uuid-dev zlib1g-dev libpulse-dev gcc gcc-multilib  g++ g++-multilib cmake linux-headers-$(uname -r) build-essential unixodbc-dev
```
#### Ubuntu 32-bit
```sh
sudo dpkg --add-architecture i386
sudo apt update 
sudo apt install unixodbc-dev:i386 odbcinst1debian2:i386 libodbc1:i386 libcurl4-openssl-dev:i386 libssl-dev:i386 uuid-dev:i386 cpp:i386 cpp-9:i386 gcc:i386 g++:i386 zlib1g-dev:i386 linux-headers-$(uname -r) gcc-multilib:i386 g++-multilib:i386 cmake g++-9:i386 gcc-9:i386 gcc-9-multilib:i386 g++-9-multilib:i386 binutils:i386 make:i386
```
#### Amazon Linux 2 64-bit
```sh
# Install dependencies
sudo yum update
sudo yum install curl-devel openssl-devel uuid-devel zlib-devel pulseaudio-libs-devel kernel-devel gcc gcc-c++ unixODBC-devel rpm-build

# Download and build CMake 3
wget https://cmake.org/files/v3.18/cmake-3.18.0.tar.gz
tar -xvzf cmake-3.18.0.tar.gz
cd cmake-3.18.0
./bootstrap
make -j4
sudo make install
cd ..
```

### Building the Driver

From the Unix shell, run one of the following:
* `./build_linux_debug64.sh`
* `./build_linux_release32_deb.sh`
* `./build_linux_release64_deb.sh`
* `./build_linux_release32_rpm.sh`
* `./build_linux_release64_rpm.sh`

**Notes:**
* Ubuntu 20.04 can be used to build either the 32-bit or 64-bit deb files.
* Ubuntu 18.04 can be used to build only the 64-bit deb file.
* Amazon Linux 2 should be used to build the 64-bit rpm file.
* The 32-bit rpm distributable is not supported at this time.

### Output Location on Linux

Compiling on Linux will output the tests to **bin** and the driver to **lib**. There are also some additional test infrastructure files which output to the **lib** directory. Both 32-bit and 64-bit builds output here.

### Packaging

From the <cmake-build64> or the <cmake-build32> type:
>cpack .

An installer named `AmazonTimestreamODBC-<version>.<installer_type>` will be generated in the build directory (for example `AmazonTimestreamODBC_1.0.0_x86_64.rpm` will be created in `make-build64`).
