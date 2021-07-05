# Amazon Timestream ODBC Driver Build Instructions

## Windows

### Dependencies

* [cmake](https://cmake.org/install/)
* [Visual Studio 2019](https://visualstudio.microsoft.com/vs/) (Other versions may work, but only 2019 has been tested)
* [Amazon Timestream ODBC Driver source code](https://github.com/opendistro-for-elasticsearch/sql/tree/master/sql-odbc)

### Build

#### with Visual Studio

Run `./build_win_<config><bitness>.ps1` to generate a VS2019 project for building/testing the driver. (the build may fail, but should still generate a `.sln` file)

The solution can be found at `<odbc-root>\build\odbc\build\global_make_list.sln`.

#### with Developer Powershell

Use `./build_win_<config><bitness>.ps1` to build the driver from a Developer Powershell prompt.

> A shortcut is installed on your system with Visual Studio (search for **"Developer Powershell for VS 2019"**)

> Programs launched with this prompt (ex: VS Code) also have access to the Developer shell.

### Build Output

```
build
└-- <Configuration><Bitness>
  └-- odbc
    └-- bin
      └-- <Configuration>
    └-- build
    └-- lib
  └-- aws-sdk
    └-- build
    └-- install
```

* Driver DLL: `.\build\<Config><Bitness>\odbc\bin\<Config>\odfesqlodbc.dll`
* Test binaries folder: `.\build\<Config><Bitness>\odbc\bin\<Config>`

### Packaging

From a Developer Powershell, run:
```
msbuild .\build\Release<Bitness>\odbc\PACKAGE.vcxproj -p:Configuration=Release
```

An installer named as `AmazonTimestreamODBC<Bitness>-<version>.msi` will be generated in the build directory.


## Mac

### Dependencies

Homebrew must be installed to manage packages, to install homebrew see the [homebrew homepage](https://brew.sh/).
Using homebrew, install the following packages using the command provided:
>brew install [package]
>
>* curl
>* cmake
>* libiodbc

### Building the Driver

From a Bash shell:

`./build_mac_<config><bitness>.sh`

### Output Location on Mac

Compiling on Mac will output the tests to **bin64** and the driver to **lib64**. There are also some additional test infrastructure files which output to the **lib64** directory.

### Packaging

Run below command from the project's build directory.
>cpack .

Installer named as `AmazonTimestreamODBC-<version>.pkg` will be generated in the build directory.


## Linux

### Dependencies

The terminal can be used to install all the dependencies for Linux.

On 64-bit, the following commands can be used to collect the required dependencies.
>sudo apt update
>sudo apt install libcurl4-openssl-dev libssl-dev uuid-dev zlib1g-dev libpulse-dev linux-headers-$(uname -r) gcc gcc-multilib  g++ g++-multilib cmake linux-headers-$(uname -r) build-essential unixodbc-dev

On 32-bit, the following commands can be used to collect the required dependencies.
>sudo dpkg --add-architecture i386
>sudo apt update 
>sudo apt install unixodbc-dev:i386 odbcinst1debian2:i386 libodbc1:i386 libcurl4-openssl-dev:i386 libssl-dev:i386 uuid-dev:i386 cpp:i386 cpp-9:i386 gcc:i386 g++:i386 zlib1g-dev:i386 linux-headers-$(uname -r) gcc-multilib:i386 g++-multilib:i386 cmake g++-9:i386 gcc-9:i386 gcc-9-multilib:i386 g++-9-multilib:i386 binutils:i386 make:i386

### Building the Driver

From a terminal, execute:

`./build_linux_<config><bitness>_<installer_type>.sh`

### Output Location on Linux

Compiling on Linux will output the tests to **bin64** and the driver to **lib64**. There are also some additional test infrastructure files which output to the **lib64** directory.

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
