# ODBC sample client application - ODBCcli

A simple odbc client application that lets you pass a dsn and query to run on a Timestream database.

## Prerequisites
- [CMake](https://cmake.org/)

## Supported platforms
- Windows - Tested in Windows 10 only
- MacOS - Tested. Need [iODBC](http://www.iodbc.org/dataspace/doc/iodbc/wiki/iodbcWiki/WelcomeVisitors) setup
- unixODBC - Tested. Need [unixODBC](http://www.unixodbc.org/) setup

## Compiling with make
```
$ cd ODBCcli
$ mkdir build
$ cd build
$ cmake ../ -DCMAKE_BUILD_TYPE=Release
$ make
```
## Compiling with Visual Studio 2019
- cmake needs to be run to generate the .vcxproj file
```
1. Open the command prompt in Administrator mode
2. Navigate to the direcory where the project was created (e.g. Documents\timestream-odbc\tools\ODBCcli)
3. Create the build directory (mkdir build).
4. Navigate to the build directory (cd build).
5. Run cmake to generate the .vcxproj file (cmake ../ -DCMAKE_BUILD_TYPE=Release)
6. Open Visual Studio 2019 and open the ODBCcli.vcxproj file.
7. Select the appropriate build type (debug or release).
8. Right click on ODBCcli and click Build.
9. odbccli.exe will be created in the Build\Debug directory or Build\Release directory depending on the version built
```
## Running the Sample Application - ODBCcli
On Windows:
```
ODBCcli.exe <your-connection-string> <your-query>
```
For example:
```
ODBCcli.exe "DSN=timestream-aws-profile" "SELECT * FROM CLIExample.ExampleTable LIMIT 1"
```
On macOS or Linux:
```
./ODBCcli <your-connection-string> <your-query>
```
For example:
```
./ODBCcli "DSN=timestream-aws-profile" "SELECT * FROM CLIExample.ExampleTable LIMIT 1"
```