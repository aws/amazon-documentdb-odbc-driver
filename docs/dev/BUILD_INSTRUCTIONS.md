# Amazon Timestream ODBC Driver Build Instructions

## Windows
* [Building the Amazon Timestream ODBC Driver](./build_win.md)
* [Running the Amazon Timestream ODBC Driver Automated Tests](./run_tests_win.md)
* [Configuring a DSN](../user/windows_configure_dsn.md)

## Mac
* [Building the Amazon Timestream ODBC Driver](./build_mac.md)
* [Running the Amazon Timestream ODBC Driver Automated Tests](./run_tests_mac.md)
* [Configuring a DSN](../user/mac_configure_dsn.md)

## Linux
* [Building the Amazon Timestream ODBC Driver](./build_linux.md)
* [Running the Amazon Timestream ODBC Driver Automated Tests](./run_tests_linux.md)
* [Configuring a DSN](../user/linux_configure_dsn.md)

## Power BI Connector
* [Building the Power BI Connector](./build_powerbi_connector.md)

## General Build Information

### ODBC Driver CMake Options

**BUILD_WITH_TESTS**

(Defaults to ON) If disabled, all tests and test dependencies will be excluded from the build which will optimize the installer package size. This option can set with the command line (using `-D` for example `-D BUILD_WITH_TESTS=OFF`).
