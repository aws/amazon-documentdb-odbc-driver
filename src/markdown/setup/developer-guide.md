# Development Environment

## Pre-requisites

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

Other Environment Variables
1. `JAVA_HOME`=`<path-to-JDK-root-folder>` (e.g., `C:\Users\user\.jdks\corretto-17.0.6`)
2. `DOCUMENTDB_HOME`=`<path-to-bin-ouput-folder>` (e.g., `C:\Users\user\GitHub\aws\amazon-documentdb-odbc-driver\build\odbc\bin\Debug`)
3. `VCPKG_ROOT`=`<path-to-vcpkg-folder>` (e.g., `C:\VCPKG`)
4. `DOC_DB_LOCAL_PORT`=`27019` (e.g., If the local port is a value other than 27019, set `DOC_DB_LOCAL_PORT` to that value.)
5. `DOC_DB_REMOTE_PORT`=`27017` (e.g., If the remote port is a value other than 27017, set `DOC_DB_REMOTE_PORT` to that value.)

[`Optional`] Log configurations

Set these 2 variables only if you would like to set a custom log path or log level for connection tests; it is completely optional.
1. `DOC_DB_LOG_PATH`=`<path_to_log_file>`(e.g.:`"C:\\Users\\BitQuillUser\\Desktop\\DocumentDB ODBC Driver"`)

   The user needs to ensure that the directory mentioned in the log file path does exist, or driver will ignore user's passed value and create the log file in the default log path. Do **not** include a slash at the end of the log path.

   The log path indicates the path to store the log file. The log file name has `docdb_odbc_YYYYMMDD.log` format, 
   where `YYYYMMDD` (e.g., 20220225 <= Feb 25th, 2022) is the date at the first log message.

2. `DOC_DB_LOG_LEVEL`=`<log_level>`. The default is `error` level. (Choose from `debug`, `info`, `error`, `off`)

   More details about logging in [`src\markdown\troubleshooting-guide.md`](src/markdown/troubleshooting-guide.md).

### Running an SSH tunnel for Testing
By default, remote integration tests are not run. To enable remote integration tests, 
set the environment variable `DOC_DB_ODBC_INTEGRATION_TEST=1`
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
   Alternatively, you can run MongoDB in a docker container
   E.g. `docker run --name mongo -e MONGO_INITDB_ROOT_USERNAME=$DOC_DB_USER_NAME -e MONGO_INITDB_ROOT_PASSWORD=$DOC_DB_PASSWORD -d -p 27017:27017 mongo:latest`
2. Install the test data
   1. `cd scripts`
   2. `.\import_test_data.ps1` (Windows) or `./import_test_data.sh` (MacOS or Linux).
   If receiving permission errors on MacOS while importing the test data, 
   use `chmod +x ./reinstall_mongodb_mac.sh` and try again.

## Windows

1. Microsoft Visual Studio (Community 2019 Verified)
   1. Desktop Development for C++
   2. Visual Studio core editor
   3. C++ ATL for latest v142 build tools (x86 & x64)
   4. C++ MFC for latest v142 build tools (x86 & x64)
   5. [WiX Toolset v3 Schemas for Visual Studio](https://wixtoolset.org/releases/)
      If encountering "wix toolset requires .net 3.5.1 to be enabled", follow the steps below. Depending on which version of Windows OS you're running...
      1. ([Windows 10/11](https://learn.microsoft.com/en-us/dotnet/framework/install/dotnet-35-windows#enable-the-net-framework-35-in-control-panel))
      Press your Windows key (or click Start button) and search for "Turn Windows features on or off",
      and select ".Net Framework 3.5 (includes .NET 2.0 and 3.0)".
      <br>![.NET 3.5 installation](../images/dotNet_screenshot.PNG "Example")</br>
      2. ([Windows Server](https://learn.microsoft.com/en-us/windows-hardware/manufacture/desktop/enable-net-framework-35-by-using-the-add-roles-and-features-wizard?view=windows-11))
      Open Server Manager, and use the "Add Roles and Features Wizard" path. Follow the wizard steps and ensure to select ".Net Framework 3.5 Features".
   6. [WiX Toolset Visual Studio 2019 Extension](https://marketplace.visualstudio.com/items?itemName=WixToolset.WiXToolset)
   7. The selected components should look similar to the following:
   <br>![.NET 3.5 installation](../images/visual-studio-2022-components.png)</br>
2. Ensure PowerShell (64-bit, version 7 or greater) is installed.
   1. [Dowload and install PowerShell](https://docs.microsoft.com/en-us/powershell/scripting/install/installing-powershell-on-windows?view=powershell-7.2).
   2. Create a short-cut for a Developer Shell. Change the path to the `Microsoft.VisualStudio.DevShell.dll` as appropriate for
   your version of Visual Studio. 
      - Example short-cut path: `"C:\Program Files\PowerShell\7\pwsh.exe" -noe -c "&{Import-Module """C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"""; Enter-VsDevShell db62f8c1}"`
2. Ensure `chocolatey` is installed.
   1. Using the instructions from [chocolatey's installation instructions](https://chocolatey.org/install), install the utility
   via a PowerShell terminal.
2. Setup Environment Variables
   1. Start a version 7 or greater PowerShell (pwsh.exe) 
   2. Change directory to the project directory.
   3. Run the following script: `scripts\env_variables_check.ps1`
   4. For any missing environment variables, add the missing environment variable to your
   user setup.
2. Install Dependencies
   1. Start a version 7 or greater PowerShell (pwsh.exe) - preferrably 
    in ***Run as Administrator*** mode.
   2. Change directory to the project directory.
   3. Run the installation script `scripts\install_dependencies_win.ps1`. Note: ***This will
   forcibly install any dependency that has not already been set (i.e, Java JDK, libraries and local MongoDB server).***
6. On the Developer PowerShell, run one of the build scripts to create an initial compilation.
   1. E.g.: `.\build_win_debug64.ps1`
   2. Navigate to the `build\odbc\cmake` folder to use the generated solution file, `DocumentDB-ODBC.C++.sln` to work on
   source code development and testing.
8. Open a **64-bit** command shell or **64-bit** PowerShell window, ***as Administrator***, run the command below
   ```
   .\<repo-folder>\src\odbc\install\install_amd64.cmd <repo-folder>\build\odbc\cmake\Debug\documentdb.odbc.dll
   ``` 
   Ensure that backslashes are used in your command. 

## MacOS

1. Install dependencies by running `./script/install_dependencies_mac.sh`. Or you can do it manually with commands below.
   1. `brew install cmake`
   2. `brew install openssl`
   3. `brew install libiodbc`  
      - You may need to unlink `unixodbc` if you already have this installed. Use `brew unlink unixodbc`.
   4. `brew install boost`
   5. Install MongoDB using [`MongoDB's Installation Guide for MongoDB Community Edition on macOS`](https://www.mongodb.com/docs/manual/tutorial/install-mongodb-on-os-x/).
   6. `brew install mongo-cxx-driver`
   7. Install Java **JDK** (version 8+ - 17 recommended)  
      - This can be done through Homebrew using `brew install --cask corretto<version>`. 
      - If the above command fails with `No Cask with this name exists`, run `brew tap homebrew/cask-versions` first and try it again.
      - Ensure to set `JAVA_HOME`. Make sure it is set to corretto. Other JDK package may cause test errors 
      such as `Unable to get initialized JVM` at run time.  
      Example: /Library/Java/JavaVirtualMachines/amazon-corretto-18.jdk/Contents/Home
      - Ensure to save Java `/bin` and `/server` directories to the User `PATH` variable.  
      Example: /Library/Java/JavaVirtualMachines/jdk-17.0.2.jdk/Contents/Home/lib/server/
      /Library/Java/JavaVirtualMachines/jdk-17.0.2.jdk/Contents/Home/bin/
   8. If creating a debug build (`./build_mac_debug64.sh`), LLVM is required.
      - If you only have XCode Command Line Tools, use the LLVM included with XCode by modifying the PATH with `export PATH=/Library/Developer/CommandLineTools/usr/bin/:$PATH`. Ensure this XCode path comes first in $PATH. If error occurs, check that clang and llvm are under folder Library/Developer/CommandLineTools/usr/bin.
      - If you have XCode application, to ensure LLVM and CMake are compatible, use the LLVM included with XCode by modifying the PATH with `export PATH=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/:$PATH`.
2. Run one of the build scripts to create an initial compilation.
   1. E.g.: `./build_mac_release64.sh`
   2. Navigate to the `build/odbc/lib` folder to use the generated files.
3. Call `./script/set_vars_after_build_mac.sh` to set `DOCUMENTDB_HOME` and `ODBCINSTINI` automatically. Or you can do it mannually with instructions below. 
   - Set the environment variable `DOCUMENTDB_HOME`. On a developer's machine, set it to `<repo-folder>/build/odbc/bin`
   - Set the environment variable `ODBCINSTINI`. On a developer's machine, set it to `<repo-folder>/build/odbc/lib/documentdb-odbc-install.ini`.
4. Run the following command to register the ODBC driver. 
   `./scripts/register_driver_unix.sh`.
5. Now you're ready to run the tests (e.g., `./build/odbc/bin/documentdb-odbc-tests  --catch_system_errors=false`).
6. More details in [`src\DEVNOTES.txt`](src/DEVNOTES.txt).

## Linux

### Using docker

#### Pre-requisites 

1. Mongo instance running in your local (host) machine or in a docker container. Check [Running and Installing Local MongoDB Server ](#running-and-installing-local-mongodb-server)
   1. If running in a docker container collect the MongoDB container IP.
      1. Use command `docker inspect -f '{{range.NetworkSettings.Networks}}{{.IPAddress}}{{end}}' container_name_or_id` save this IP, which will be used later.
2. Build docker image
   1. Navigate Dockerfile folder `cd docker/linux-environment`
   2. Build the docker image E.g.: `docker build -t documentdb-dev-linux .`   
3. If necessary to run the integrations tests, follow the next step.
   1. Copy necessary pem files to access DocumentDB and ssh tunnel to the root of your project. See [Running an SSH Tunnel for testing](#running-an-ssh-tunnel-for-testing)
      E.g. `cp ~/.ssh/ssh-tunnel.pem <project-root-folder>`
      Alternatively you can add -v in docker run command to mount your folder with the pem files
      E.g. `-v <local path of pem folder file>:/<docker container path>` `

#### Using the dev image

1. Run docker container with interactive mode. If you are running MongoDB in the host, run `docker run --add-host host.docker.internal:host-gateway -v <local path of documentdb odbc repo>:/documentdb-odbc -it documentdb-dev-linux`. If you MongoDB is running in another conatainer run `docker run -v <local path of documentdb odbc repo>:/documentdb-odbc -it documentdb-dev-linux`
2. Next steps all are from inside the container
   1. Set environment variables for testing and double-check if all dev environmnet variables are set by running `scripts/env_variables_check.sh`. More info [Environment Variables for Testing Accounts/Secrets ](#environment-variables-for-testing-accounts/secrets)
      Note. Since the environment variables JAVA_HOME, DOCUMENTDB_HOME and ODBC_LIB_PATH are already set in the container, it is not recommended to change those.
   2. Run one of the build scripts to create an initial compilation. E.g. `./build_linux_debug64.sh` or `./build_linux_release64_deb.sh`
   3. Run the following command to register the ODBC driver. 
      `./scripts/register_driver_unix.sh`
   4. set LOCAL_DATABASE_HOST with the ip of your local mongo
      E.g. If is in another docker container `export LOCAL_DATABASE_HOST=<ip from the mongo docker container>` or if is your host machine `export LOCAL_DATABASE_HOST=host.docker.internal`
   5. You are ready to run the tests.
   E.g. `cd /documentdb-odbc/build/odbc/bin/ && ./documentdb-odbc-tests  --catch_system_errors=false`
3. More details in [`src\DEVNOTES.txt`](src/DEVNOTES.txt).

#### Known issues

If a windows host machine is used, it is possible to have an issue with end of line character in the *.sh files. 
There are two ways to fix the issue.
   1. Ensure that your github is checking out the files as Unix-style https://docs.github.com/en/get-started/getting-started-with-git/configuring-git-to-handle-line-endings
   2. Alternatively you can convert the end-of-line using the following command `tr -d '\015' < build_linux_debug64.sh > build_linux_debug64_lf.sh and run the build_linux_debug64_lf.sh`
      1. Note that the command will need to be executed for all scripts that you will run in the container (register_driver_unix.sh,env_variables_check.sh and any other that you might need).

### Using Ubuntu 64bit

1. Install all dependencies
   1. Ubuntu dev dependencies
      E.g. 
``` 
           apt-get -y update \
           && apt-get -y install wget \
                                 curl \
                                 libcurl4-openssl-dev \
                                 libssl-dev \
                                 uuid-dev \
                                 zlib1g-dev \
                                 libpulse-dev \
                                 gcc \
                                 gcc-multilib  \
                                 g++ \
                                 g++-multilib \
                                 build-essential \
                                 valgrind \
                                 libboost-all-dev \
                                 libsasl2-dev \
                                 libbson-dev \
                                 lcov \
                                 git \
                                 unixodbc-dev \
                                 valgrind 
```
   2. Compile and install mongo-cxx-driver
      E.g. 
```
           cd /tmp \
           && curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.6.6/mongo-cxx-driver-r3.6.6.tar.gz \
           && tar -xzf mongo-cxx-driver-r3.6.6.tar.gz \
           && rm mongo-cxx-driver-r3.6.6.tar.gz \
           && cd mongo-cxx-driver-r3.6.6/build \
           && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local -DBSONCXX_POLY_USE_MNMLSTC=1 .. \
           && sudo make \
           && sudo make install 
```
   3. Compile and install mongo-cxx
      E.g. 
```
          cd /tmp \
          && curl -OL https://github.com/mongodb/mongo-c-driver/releases/download/1.21.1/mongo-c-driver-1.21.1.tar.gz \
          && tar xzf mongo-c-driver-1.21.1.tar.gz \
          && rm mongo-c-driver-1.21.1.tar.gz \
          && cd mongo-c-driver-1.21.1 \
          && mkdir cmake-build \
          && cd cmake-build \
          && cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF .. \
          && sudo make \
          && sudo make install
```
   4. Install Java if necessary ( correto 17 is recommended) Follow this (link)[https://docs.aws.amazon.com/corretto/latest/corretto-17-ug/generic-linux-install.html] for instructions.
   5. Set all necessary environment variables and run the following command to register the ODBC driver. 
      `./scripts/register_driver_unix.sh`
   6. Run one of the build scripts to create an initial compilation. E.g. `./build_linux_release64.sh`
   7. Set environment variables for testing and double-check if all dev environmnet variables are set running `scripts/env_variables_check.sh`.
   8. set LOCAL_DATABASE_HOST with the ip of your local mongo
      E.g. If is in another docker container `export LOCAL_DATABASE_HOST=<ip from the mongo docker container>` or if is your host machine `export LOCAL_DATABASE_HOST=host.docker.internal`.
   9. You are ready to run the tests.
      E.g. `/documentdb-odbc/build/odbc/bin/documentdb-odbc-tests --catch_system_errors=false`.

## Code Coverage

### MacOS/Linux

To generate code coverage reports you to need to use the debug builds for macOS/Linux, run the tests and use `gcovr` to compile the report. For an installation guide and how to use it look at the official (documentation)[https://gcovr.com/en/stable/index.html].
If you want to check a detailed report generate the with `--html-details` option.

### Windows

OpenCppCoverage is used to generate code coverage for windows, for more information check it in the official (documentation)[https://github.com/OpenCppCoverage/OpenCppCoverage]

## Versioning

1. To set the version of the ODBC driver, update the `src/ODBC_DRIVER_VERSION.txt` file with the appropriate version.
2. To set the version of the JDBC drvier to download and embed, update the `src/JDBC_DRIVER_VERSION.txt` file to the appropriate version. 

## Power BI Connector

1. Install [VSCode Power Query
   SDK](https://marketplace.visualstudio.com/items?itemName=PowerQuery.vscode-powerquery-sdk) documentation
   ([reference](https://learn.microsoft.com/en-us/power-query/power-query-sdk-vs-code))
2. Open the folder `src/power-bi-connector/AmazonDocumentDBConnector` in a new VScode window. **Note**: The Power Query
   SDK will not work if you open the root directory of the Amazon DocumentDB ODBC driver

### Run Power BI Connector Tests

1. Build the project use the Terminal command ***Run build task...***, ***build: Build connector project using MakePQX***.
1. Make sure to set the crendentials before running any tests
   ([reference](https://learn.microsoft.com/en-us/power-query/power-query-sdk-vs-code#set-credential))
1. Use the ***Run TestConnection function*** to ensure the connection is valid 
   ([reference](https://learn.microsoft.com/en-us/power-query/power-query-sdk-vs-code#run-testconnection-function))
2. Select the `AmazonDocumentDBConnector.query.pq` file and click on ***Evaluate Current File***
   ([reference](https://learn.microsoft.com/en-us/power-query/power-query-sdk-vs-code#evaluate-a-query-and-the-results-panel))

## Quality Components

The project ensures quality through static and runtime components.

### Static Components

1. `cppcheck` during GitHub pull request workflow
1. `.clang-format` - enforces intendentation and C++ style for each developer.
1. Windows compiler flags 
   1. /guard:cf - [Enable Control Flow Guard](https://learn.microsoft.com/en-us/cpp/build/reference/guard-enable-control-flow-guard)
   1. /GS - [Buffer Security Check](https://learn.microsoft.com/en-us/cpp/build/reference/gs-buffer-security-check)
   1. /analyze - [Code analysis](https://learn.microsoft.com/en-us/cpp/build/reference/analyze-code-analysis)
   1. /DYNAMICBASE - [Use address space layout randomization](https://learn.microsoft.com/en-us/cpp/build/reference/dynamicbase-use-address-space-layout-randomization)
   1. /WX - [Treat compiler warnings as errors](https://learn.microsoft.com/en-us/cpp/build/reference/compiler-option-warning-level)
1. Windows linker flags
   1. /SAFESEH - [Image has Safe Exception Handlers](https://learn.microsoft.com/en-us/cpp/build/reference/safeseh-image-has-safe-exception-handlers)
   1. /NXCOMPAT - [Compatible with Data Execution Prevention](https://learn.microsoft.com/en-us/cpp/build/reference/nxcompat-compatible-with-data-execution-prevention)
   1. /WX - [Treat linker warnings as errors](https://learn.microsoft.com/en-us/cpp/build/reference/wx-treat-linker-warnings-as-errors)
1. ACAT scan - run manually before any release.
1. `trufflehog` - run during GitHub workflow to detect possible passwords or secrets in source code.

### Runtime Components

1. `code coverage` - checks during GitHub pull request workflow
1. `boost` - test platform framework.
   1. The source project is `odbc-test`.
   1. Test platform for unit and integration tests.
   1. Checks for memory leaks and unhandled runtime exceptions.
