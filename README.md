# Amazon DocumentDB ODBC Driver

## Development Environment

### Windows

1. Microsoft Visual Studio (Community 2019 Verified)
   1. Desktop Development for C++
   2. Visual Studio core editor
   3. C++ ATL for latest v142 build tools (x86 & x64)
   4. C++ MFC for latest v142 build tools (x86 & x64)
   5. WiX Toolset v3 Schemas for Visual Studio
   6. WiX Toolset Visual Studio 2019 Extension
2. OpenSSL (full)
   1. Installed via [VCPKG](https://vcpkg.io/en/getting-started.html) (`.\vcpkg install openssl`).
   2. Or installed via [Chocolatey](https://community.chocolatey.org/packages/openssl). 
   3. Ensure to set the OPENSSL_ROOT_DIR.
3. [WiX Installer (3.11)](https://wixtoolset.org/releases/)
   1. Ensure to add path to WiX executables (e.g. `C:\Program Files (x86)\WiX Toolset v3.11\bin`)
4. Java **JDK** (version 8+ - 17 recommended)
   1. Ensure to set `JAVA_HOME`. 
5. Boost Test Framework 
   1. Install via [VCPKG](https://vcpkg.io/en/getting-started.html) using `.\vcpkg install openssl:x64-windows boost-test:x64-windows boost-asio:x64-windows boost-chrono:x64-windows boost-interprocess:x64-windows boost-regex:x64-windows boost-system:x64-windows boost-thread:x64-windows`
6. Run one of the build scripts to create an initial compilation.
   1. E.g.: `.\build_win_debug64.ps1`
   2. Navigate to the `build\odbc\cmake` folder to use the generated solution file, `Ignite.C++.sln` to work on
   source code development and testing.
7. More details in `src\DEVNOTES.txt`.

### MacOS

1. Install dependencies
   1. `brew install cmake`
   2. `brew install openssl`
   3. `brew install unixodbc`  
      - You may need to unlink `libiodbc` if you already have this installed. Use `brew unlink libiodbc`.
   4. `brew install boost`
   5. Install Java **JDK** (version 8+ - 17 recommended)  
      - This can be done through Homebrew using `brew install --cask temurin<version>`. 
      - Ensure to set `JAVA_HOME`.  
   6. If creating a debug build (`./build_mac_debug64.sh`), LLVM is required.
      - If you only have XCode Command Line Tools, use the LLVM included with XCode by modifying the PATH with `export PATH=/Library/Developer/CommandLineTools/usr/bin/:$PATH`. Ensure this XCode path comes first in $PATH. If error occurs, check that clang and llvm are under folder Library/Developer/CommandLineTools/usr/bin.
      - If you have XCode application, to ensure LLVM and CMake are compatible, use the LLVM included with XCode by modifying the PATH with `export PATH=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/:$PATH`.
2. Run one of the build scripts to create an initial compilation.
   1. E.g.: `./build_mac_release64.sh`
   2. Navigate to the `build/odbc/lib` folder to use the generated files.
3. More details in `src\DEVNOTES.txt`.

### Linux

TBD

### Troubleshooting 

#### Issue: MacOS build fails with error about iODBC header
##### Example error message  
```
/Library/Frameworks/iODBC.framework/Headers/sqlext.h:82:10: fatal error: 'iODBC/sql.h' file not found
#include <iODBC/sql.h>
     ^~~~~~~~~~~~~ 
``` 
##### Fix 
If you have installed the iODBC Driver Manager, the headers installed with it might be used instead of those from `unixodbc`. You may need to uninstall this driver manager and remove the `/Library/Frameworks/iODBC.framework/` directory. 
