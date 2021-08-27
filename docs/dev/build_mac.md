# macOS - Building the Amazon Timestream ODBC Driver

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
