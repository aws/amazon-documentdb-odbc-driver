# ODBCCli

It is a very simple odbc client application to support fuzzy testing of an ODBC driver.

## Prerequisite
- [CMake](https://cmake.org/)

## Support platform
- Tested in Windows 10 only
- Need [iODBC](http://www.iodbc.org/dataspace/doc/iodbc/wiki/iodbcWiki/WelcomeVisitors) setup for macOS (Not tested!)
- Need [unixODBC](http://www.unixodbc.org/) setup for unixODBC (Not tested!)

## How to run it?
```
$ cd ODBCcli
$ mkdir build
$ cd build
$ cmake ../ -DCMAKE_BUILD_TYPE=Release
$ make
$ ODBCCli.exe
```