# Amazon DocumentDB ODBC Driver

## Development Environment

### Windows

1. Microsoft Visual Studio (Community 2019 Verified)
   1. Desktop Development for C++
   2. Visual Studio core editor
   3. C++ ATL for latest v142 build tools (x86 & x64)
   4. C++ MFC for latest v142 build tools (x86 & x64)
   5. WiX Toolset v3 Schemas for Visual Studio
   6. Wix Toolset v4 Schemas for Visual Studio
   7. WiX Toolset Visual Studio 2019 Extension
2. OpenSSL (full)
   1. Installed via [Chocolatey](https://community.chocolatey.org/packages/openssl).
   2. Or installed via [VCPKG](https://vcpkg.io/en/getting-started.html) (`.\vcpkg install openssl`).
   3. Ensure to set the OPENSSL_ROOT_DIR.
3. [WiX Installer (3.11)](https://wixtoolset.org/releases/)
   1. Ensure to add path to WiX executables (e.g. `C:\Program Files (x86)\WiX Toolset v3.11\bin`)
4. Java **JDK** (version 8+ - 17 recommended)
   1. Ensure to set JAVA_HOME
5. Run one of build scripts to create an initial compilation.
   1. E.g.: `.\build_win_debug64.ps1`
   2. Navigate to the `build\odbc\cmake` folder to use the generated solution file, `Ignite.C++.sln` to work on
   source code development and testing.
6. More details in `src\DEVNOTES.txt`.

### MacOS

1. Install CMAKE
   1. brew install cmake
2. Run one of build scripts to create an initial compilation.
   1. E.g.: `./buid_mac_release64.sh`
   2. Navigate to the `build/odbc/lib` folder to use the generated files.
3. Use Microsoft VS Code to work on source code devlopment and testing.

### Linux

TBD
