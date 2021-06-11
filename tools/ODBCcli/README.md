# ODBC sample client application - ODBCCli

A simple odbc client application that lets you pass a dsn and query to run on a Timestream database.

## Prerequisite
- [CMake](https://cmake.org/)

## Support platform
- Windows - Tested in Windows 10 only
- MacOS - Tested. Need [iODBC](http://www.iodbc.org/dataspace/doc/iodbc/wiki/iodbcWiki/WelcomeVisitors) setup
- unixODBC - Not tested. Need [unixODBC](http://www.unixodbc.org/) setup

## How to run it?
```
$ cd ODBCcli
$ mkdir build
$ cd build
$ cmake ../ -DCMAKE_BUILD_TYPE=Release
$ make
```
If it is in Windows, use 
```
ODBCcli.exe <your-dsn> <your-query>
```
If it is in macOS, use
```
./ODBCcli <your-dsn> <your-query>
```
