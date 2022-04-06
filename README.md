# Amazon DocumentDB ODBC Driver

## Development Environment

### Pre-requisites

#### C/C++ Formatting

- This project uses [Google's C++ Style Guide](https://google.github.io/styleguide/cppguide.htm) as a basis for 
C/C++ usage and formatting.
- Some formatting is set using the .clang-format file at the base of repository. Other options for Visual Studio can be imported from the 
`VS-C++-Settings-Export.vssettings` file also found at root of repository.

#### Environment Variables for Testing Accounts/Secrets 
To enable the test environment to run the tests against a live DocumentDB system, set the following environment variables on your development machine.

DocumentDB cluster credentials
1. `DOC_DB_HOST`=`<remote_documentdb_host>`(e.g.:`docdb-host.us-east-2.docdb.amazonaws.com`)
2. `DOC_DB_PASSWORD`=`<documentdb_user_password>`
3. `DOC_DB_USER_NAME`=`<documentdb_user_name>`

SSH host credentials 
1. `DOC_DB_USER`=`<ssh_user>`(e.g.:`ec2-user@ec2-instance.us-east-2.compute.amazonaws.com`)
2. `DOC_DB_PRIV_KEY_FILE`=`<path_to_ssh_host_private_key_file>`(e.g.:`~/.ssh/ssh_host.pem`)

#### Running an SSH tunnel for Testing
By default, remote integration tests are not run. To enable remote integration tests, 
set the environment variable `DOC_DB_ODBC_INTEGRATION_TEST=1`
To run tests that require an external SSH tunnel, you will need to start an SSH tunnel using the same values as the environment variables set in the previous section. 
If the local port is a value other than 27019, set `DOC_DB_LOCAL_PORT` to that value. 
If the remote port is a value other than 27017, set `DOC_DB_REMOTE_PORT` to that value. 

Example:
```
 ssh -i $DOC_DB_PRIV_KEY_FILE -N -L $DOC_DB_LOCAL_PORT:$DOC_DB_HOST:$DOC_DB_REMOTE_PORT $DOC_DB_USER
```

#### Running and Installing Local MongoDB Server 

1. Run the following script to setup MongoDB server on your machine.
   1. `cd src/odbc-test/scripts`
   2. `.\reinstall_mongodb.ps1` (Windows) or `./reinstall_mongodb_mac.sh` (MacOS)
   Alternativetly, you can run MongoDB in a docker container
   E.g. `docker run --name mongo -e MONGO_INITDB_ROOT_USERNAME=$DOC_DB_USER_NAME -e MONGO_INITDB_ROOT_PASSWORD=$DOC_DB_PASSWORD -d -p 27017:27017 mongo:latest`
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
   `./scripts/register_driver_unix.sh`
5. More details in [`src\DEVNOTES.txt`](src/DEVNOTES.txt).

### Linux

#### Using docker

##### Pre-requisites 

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

##### Using the dev image

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
   E.g. `cd /documentdb-odbc/build/odbc/bin/ && ./ignite-odbc-tests`
3. More details in [`src\DEVNOTES.txt`](src/DEVNOTES.txt).

##### Known issues

If a windows host machine is used, it is possible to have an issue with end of line character in the *.sh files. 
There are two ways to fix the issue.
   1. Ensure that your github is checking out the files as Unix-style https://docs.github.com/en/get-started/getting-started-with-git/configuring-git-to-handle-line-endings
   2. Alternatively you can convert the end-of-line using the following command `tr -d '\015' < build_linux_debug64.sh > build_linux_debug64_lf.sh and run the build_linux_debug64_lf.sh`
      1. Note that the command will need to be executed for all scripts that you will run in the container (register_driver_unix.sh,env_variables_check.sh and any other that you might need).
#### Using Ubuntu 64bit

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
   2. Compile and install mongoc
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
   4. Install Java if necessary ( correto 17 is recommended) Follow the link
   5. Set all necessary environment variables and run the following command to register the ODBC driver. 
      `./scripts/register_driver_unix.sh`
   6. Run one of the build scripts to create an initial compilation. E.g. `./build_linux_release64.sh`
   7. Set environment variables for testing and double-check if all dev environmnet variables are set running `scripts/env_variables_check.sh`.
   8. set LOCAL_DATABASE_HOST with the ip of your local mongo
      E.g. If is in another docker container `export LOCAL_DATABASE_HOST=<ip from the mongo docker container>` or if is your host machine `export LOCAL_DATABASE_HOST=host.docker.internal`.
   9. You are ready to run the tests.
      E.g. `/documentdb-odbc/build/odbc/bin/ignite-odbc-tests`.



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
