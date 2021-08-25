# Amazon Timestream ODBC Driver Build Instructions

## Windows

### Dependencies

* [Visual Studio 2019](https://visualstudio.microsoft.com/vs/) (Other versions may work, but only 2019 has been tested)
** Install Desktop development with C++
** check “C++ MFC for latest vXXX build tools”
** Install CLangFormat Extension from the Extensions menu in Visual Studio or from the [Marketplace](https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.ClangFormat).
* [Amazon Timestream ODBC Driver source code](https://github.com/Bit-Quill/timestream-odbc)
* [cmake](https://cmake.org/install/). This is installed with Visual Studio.
** Ensure that the PATH environment variable contains `C:\Program Files\Git\cmd`.
* Turn on Microsoft .NET Framework 3.5.1 on Windows 2008
* [Wix Toolset](https://wixtoolset.org/releases/)

### Build

Using Developer Powershell, run one of the four powershell files to generate the build output. It also generates the VS2019 solution for building/testing the driver at `.\build\odbc\cmake\global_make_list.sln`. 

* `.\build_win_debug32.ps1`
* `.\build_win_release32.ps1`
* `.\build_win_debug64.ps1`
* `.\build_win_release64.ps1`

### Build Output

```
build
└-- odbc
  └-- bin
    └-- <Configuration>
  └-- cmake
  └-- lib
    └-- <Configuration>
└-- aws-sdk
  └-- build
  └-- install
```

* Driver DLL: `.\build\odbc\bin\<Configuration>\timestreamsqlodbc.dll`
* Test binaries folder: `.\build\odbc\bin\<Configuration>`

### Packaging

From Developer Powershell, run:
```
msbuild .\build\odbc\cmake\PACKAGE.vcxproj -p:Configuration=Release
```

An installer named `AmazonTimestreamODBC<Bitness>-<version>.msi` will be generated in the `.\build\cmake` directory.

### Troubleshooting
If you are changing the bitness (32-bit to/from 64-bit), you may need to delete the build directory and rebuild.

## Mac

### Dependencies

Homebrew must be installed to manage packages, to install homebrew see the [homebrew homepage](https://brew.sh/).
Using homebrew, install the following packages using the command provided:
>brew install [package]
>
>* curl
>* cmake
>* libiodbc
>* llvm (for debug version)

### Building the Driver

From Terminal, run one of the following:
* `./build_mac_debug64.sh`
* `./build_mac_release64.sh`

### Output Location on Mac

* Driver: `./build/odbc/lib/libtimestreamsqlodbc.dylib`
* Test binaries folder: `./build/odbc/bin`. Some additional test infrastructure files are also output to the **lib** directory.

### Packaging

Run below command from the project's build directory.
*  `cmake ../src`
*  `make`
*  `cpack .`

Installer named as `AmazonTimestreamODBC-<version>.pkg` will be generated in the build directory.

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
sudo yum install curl-devel openssl-devel uuid-devel zlib-devel pulseaudio-libs-devel kernel-devel gcc gcc-c++ unixODBC-devel

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

## General Build Info

### ODBC Driver CMake Options

**BUILD_WITH_TESTS**

(Defaults to ON) If disabled, all tests and test dependencies will be excluded from build which will optimize the installer package size. This option can set with the command line (using `-D` for example `-D BUILD_WITH_TESTS=OFF`).

### Setting up a DSN

A **D**ata **S**ouce **N**ame is used to store driver information in the system. By storing the information in the system, the information does not need to be specified each time the driver connects.

#### Windows
The DSN can be setup for Windows using the 32-bit or 64-bit version of the ODBC database manager.

> If not using the ODBC database manager, add the following keys in the Registry
>
   >* **HKEY_LOCAL_MACHINE/SOFTWARE/ODBC/ODBC.INI** : Contains a key for each Data Source Name (DSN)
   >* **HKEY_LOCAL_MACHINE/SOFTWARE/ODBC/ODBC.INI/ODBC Data Sources** : Lists the data sources
   >* **HKEY_LOCAL_MACHINE/SOFTWARE/ODBC/ODBCINST.INI** :  Define each driver's name and setup location
   >* **HKEY_LOCAL_MACHINE/SOFTWARE/ODBC/ODBCINST.INI/ODBC Drivers** : Lists the installed drivers.
>
>These keys can be added manually in the Registry Editor (Start > Run > Regedit) one by one. Alternatively, keys can be added together as follows:
>
>1. Modify the appropriate values for these keys in `src/IntegrationTests/ITODBCConnection/test_dsn.reg`
>2. Double click on the `test_dsn.reg` file.
>3. Click `Yes` on the confirmation window to add keys in the registry.
