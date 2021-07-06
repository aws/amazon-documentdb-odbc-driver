# ODBC sample client application - ODBCCli

A simple odbc client application that lets you pass a dsn and query to run on a Timestream database.

## Prerequisite
- [CMake](https://cmake.org/)

## Support platform
- Windows - Tested in Windows 10 only
- MacOS - Tested. Need [iODBC](http://www.iodbc.org/dataspace/doc/iodbc/wiki/iodbcWiki/WelcomeVisitors) setup
- unixODBC - Not tested. Need [unixODBC](http://www.unixodbc.org/) setup

## How to compile using make
```
$ cd ODBCcli
$ mkdir build
$ cd build
$ cmake ../ -DCMAKE_BUILD_TYPE=Release
$ make
```
## How to compile using Visual Studio 2019
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
## How to run it?
In Windows:
```
ODBCcli.exe <your-dsn> <your-query>
```
In macOS:
```
./ODBCcli <your-dsn> <your-query>
```
In Linux:
```
./ODBCcli <your-dsn> <your-query>
```
