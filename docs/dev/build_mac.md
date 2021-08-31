# macOS - Building the Amazon Timestream ODBC Driver

### Dependencies

Homebrew must be installed to manage packages, to install homebrew see the [homebrew homepage](https://brew.sh/).

From Terminal run the following:
```
brew install curl
brew install cmake
brew install libiodbc
```

For debugging also run the following:
```
brew install llvm
```

### Building the Driver

From Terminal, run one of the following:
* `./build_mac_debug64.sh`
* `./build_mac_release64.sh`

### Output Location on Mac

* Driver: `./build/odbc/lib/libtimestreamsqlodbc.dylib`
* Test binaries folder: `./build/odbc/bin`. Some additional test infrastructure files are also output to the **lib** directory.

### Packaging

From the `cmake-build64` or the `cmake-build32` directory, run:

```
cmake ../src
make
cpack .
```

An installer named `AmazonTimestreamODBC-<version>.pkg` will be generated in the build directory (for example `AmazonTimestreamODBC_1.0.0.pkg` ).
