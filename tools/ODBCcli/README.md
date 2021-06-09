# ODBC sample client application - ODBCCli

It is a very simple odbc client application.

## Prerequisite
- [CMake](https://cmake.org/)

## Support platform
- Tested in Windows 10 only
- Need [iODBC](http://www.iodbc.org/dataspace/doc/iodbc/wiki/iodbcWiki/WelcomeVisitors) setup for macOS (Tested!)
- Need [unixODBC](http://www.unixodbc.org/) setup for unixODBC (Not tested!)

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
