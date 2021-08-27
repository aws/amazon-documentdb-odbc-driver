# Linux - Building the Amazon Timestream ODBC Driver - TO DO

## Linux

### Dependencies

The terminal can be used to install all the dependencies for Linux.

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

From a terminal, execute:

`./build_linux_<config><bitness>_<installer_type>.sh`

### Output Location on Linux

Compiling on Linux will output the tests to **bin** and the driver to **lib**. There are also some additional test infrastructure files which output to the **lib** directory. Both 32-bit and 64-bit builds output here.

### Packaging

Run below command from the project's build directory.
>cpack .

Installer named as `AmazonTimestreamODBC-<version>.<installer_type>` will be generated in the build directory.
