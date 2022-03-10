# Amazon DocumentDB ODBC Driver

## Development Environment

### C/C++ Formatting

- This project uses [Google's C++ Style Guide](https://google.github.io/styleguide/cppguide.htm) as a basis for 
C/C++ usage and formatting.
- Some formatting is set using the .clang-format file at the base of repository. Other options for Visual Studio can be imported from the 
`VS-C++-Settings-Export.vssettings` file also found at root of repository.

### Environment Variables for Testing Accounts/Secrets 
To enable the test environment to run the tests against a live DocumentDB system, set the following environment variables on your development machine.

DocumentDB cluster credentials
1. `DOC_DB_HOST`=`<remote_documentdb_host>`(e.g.:`docdb-host.us-east-2.docdb.amazonaws.com`)
2. `DOC_DB_PASSWORD`=`<documentdb_user_password>`
3. `DOC_DB_USER_NAME`=`<documentdb_user_name>`

SSH host credentials 
1. `DOC_DB_USER`=`<ssh_user>`(e.g.:`ec2-user@ec2-instance.us-east-2.compute.amazonaws.com`)
2. `DOC_DB_PRIV_KEY_FILE`=`<path_to_ssh_host_private_key_file>`(e.g.:`~/.ssh/ssh_host.pem`)

### Running an SSH tunnel for Testing
To run tests that require an external SSH tunnel, you will need to start an SSH tunnel using the same values as the environment variables set in the previous section. 
If the local port is a value other than 27019, set `DOC_DB_LOCAL_PORT` to that value. 
If the remote port is a value other than 27017, set `DOC_DB_REMOTE_PORT` to that value. 

Example:
```
 ssh -i $DOC_DB_PRIV_KEY_FILE -N -L $DOC_DB_LOCAL_PORT:$DOC_DB_HOST:$DOC_DB_REMOTE_PORT $DOC_DB_USER
```

### Running and Installing Local MongoDB Server 

1. Run the following script to setup MongoDB server on your machine.
   1. `cd src/odbc-test/scripts`
   2. `.\reinstall_mongodb.ps1` (Windows) or `./reinstall_mongodb_mac.sh` (MacOS)
2. Install the test data
   1. `cd src/odbc-test/scripts`
   2. `.\import_test_data.ps1` (Windows) or `./import_test_data.sh` (MacOS or Linux)
   If receiving permission errors on MacOS while importing the test data, 
   use `chmod +x ./reinstall_mongodb_mac.sh` and try again.

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
4. [Java](https://www.oracle.com/java/technologies/downloads/) **JDK** (version 8+ - 17 recommended)
   1. Ensure to set `JAVA_HOME`. (e.g. C:\Program Files\Java\jdk-17.0.2)
   2. Ensure to save Java `\bin` and `\server` directories to the User `PATH` variable. 
   Example: C:\Program Files\Java\jdk1.8.0_321\jre\bin\server
5. Boost Test Framework and Mondodb Driver
   1. Install via [VCPKG](https://vcpkg.io/en/getting-started.html) using `.\vcpkg install openssl:x64-windows boost-test:x64-windows boost-asio:x64-windows boost-chrono:x64-windows boost-interprocess:x64-windows boost-regex:x64-windows boost-system:x64-windows boost-thread:x64-windows mongo-cxx-driver:x64-windows`
6. Run one of the build scripts to create an initial compilation.
   1. E.g.: `.\build_win_debug64.ps1`
   2. Navigate to the `build\odbc\cmake` folder to use the generated solution file, `Ignite.C++.sln` to work on
   source code development and testing.
7. Set the environment variable `DOCUMENTDB_HOME`. On a developer's machine, set it to `<repo-folder>\build\odbc\bin\Debug`. The 
   build script run above, downloads it to the `<repo-folder>\build\odbc\bin\Debug\libs` folder.
8. Open a **64-bit** command shell or **64-bit** PowerShell window, **as Administrator**, run the 
   ```
   <repo-folder>\src\odbc\src\install\install_amd64.cmd <repo-folder>\buildbuild\odbc\cmake\Debug\ignite.odbc.dll
   ``` 
9. More details in [`src\DEVNOTES.txt`](src/DEVNOTES.txt).

### MacOS

1. Install dependencies
   1. `brew install cmake`
   2. `brew install openssl`
   3. `brew install unixodbc`  
      - You may need to unlink `libiodbc` if you already have this installed. Use `brew unlink libiodbc`.
   4. `brew install boost`
   5. `brew install mongodb-community`
   6. `brew install mongo-cxx-driver`
   7. Install Java **JDK** (version 8+ - 17 recommended)  
      - This can be done through Homebrew using `brew install --cask temurin<version>`. 
      - Ensure to set `JAVA_HOME`. Make sure it is set to temurin. Other JDK package may cause test errors 
      such as `Unable to get initialized JVM` at run time.  
      Example: /Library/Java/JavaVirtualMachines/temurin-17.jdk/Contents/Home
      - Ensure to save Java `/bin` and `/server` directories to the User `PATH` variable.  
      Example: /Library/Java/JavaVirtualMachines/jdk-17.0.2.jdk/Contents/Home/lib/server/
      /Library/Java/JavaVirtualMachines/jdk-17.0.2.jdk/Contents/Home/bin/
   8. If creating a debug build (`./build_mac_debug64.sh`), LLVM is required.
      - If you only have XCode Command Line Tools, use the LLVM included with XCode by modifying the PATH with `export PATH=/Library/Developer/CommandLineTools/usr/bin/:$PATH`. Ensure this XCode path comes first in $PATH. If error occurs, check that clang and llvm are under folder Library/Developer/CommandLineTools/usr/bin.
      - If you have XCode application, to ensure LLVM and CMake are compatible, use the LLVM included with XCode by modifying the PATH with `export PATH=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/:$PATH`.
2. Run one of the build scripts to create an initial compilation.
   1. E.g.: `./build_mac_release64.sh`
   2. Navigate to the `build/odbc/lib` folder to use the generated files.
3. Set the environment variable `DOCUMENTDB_HOME`. On a developer's machine, set it to `<repo-folder>/build/odbc/bin`
4. Run the following command to register the ODBC driver. 
   `./scripts/register_driver_macos.sh`
5. More details in [`src\DEVNOTES.txt`](src/DEVNOTES.txt).

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
