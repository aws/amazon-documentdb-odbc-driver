# Windows - Building the Amazon Timestream ODBC Driver

## Dependencies

* [Visual Studio 2019](https://visualstudio.microsoft.com/vs/) (Other versions may work, but only 2019 has been tested)
  * Install Desktop development with C++
  * Check "C++ MFC for latest vXXX build tools"
  * Install ClangFormat Extension from the Extensions menu in Visual Studio or from the [Marketplace](https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.ClangFormat).
* [Amazon Timestream ODBC Driver source code](https://github.com/Bit-Quill/timestream-odbc)
* [Git](https://git-scm.com/download/win)
  * Ensure that the PATH environment variable contains the path to git executable. Default is `C:\Program Files\Git\cmd`.
* [CMake](https://cmake.org/install/). This is installed with Visual Studio.
* Turn on Microsoft .NET Framework 3.5.1 on Windows 2008
* [Wix Toolset](https://wixtoolset.org/releases/)

## Build

Using Developer Powershell, navigate to the project source directory, run one of the four powershell scripts to generate the build output. It also generates the VS2019 solution for building/testing the driver at `.\build\odbc\cmake\global_make_list.sln`. 

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

From Developer Powershell, navigate to the project source directory, run:
```
msbuild .\build\odbc\cmake\PACKAGE.vcxproj -p:Configuration=Release
```

An installer named `AmazonTimestreamODBC<Bitness>-<version>.msi` will be generated in the `.\build\cmake` directory (for example `AmazonTimestreamODBC64-1.0.0.msi` ).

### Troubleshooting
If you are changing the bitness (32-bit to/from 64-bit), you may need to delete the build directory and rebuild.
