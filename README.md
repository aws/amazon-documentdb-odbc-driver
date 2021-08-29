# Amazon Timestream ODBC Driver

TimestreamODBC is a read-only ODBC driver for Windows, Mac and Linux for connecting to Amazon Timestream.

[![codecov](https://codecov.io/gh/Bit-Quill/timestream-odbc/branch/develop/graph/badge.svg?token=MNHPEGAK9D)](https://codecov.io/gh/Bit-Quill/timestream-odbc)

## Specifications

The driver is compatible with ODBC 3.51.

## Supported Versions


  | Operating System  | Version | Supported Bitness |
  | ------------- |-------------| ----------------- |
  |  Windows    |  Windows Server 2019, Windows 10  | 32-bit, 64-bit |
  |  MacOS    |  Catalina, Big Sur | 64-bit |
  |  Linux    |  Debian distro, Redhat like distro (Amazon Linux 2) |  32-bit, 64-bit  |

## Installing the Driver

You can use the installers generated from the most recent release

### Installing on Windows 10 or Windows Server 2019

1. Run `AmazonTimestreamODBC32-[version].msi` or `AmazonTimestreamODBC64-[version].msi` to install the Amazon Timestream ODBC Driver.
2. [Configure DSN entries](./docs/user/windows_configure_dsn.md) using ODBC Data Source Administrator.

### Installing on macOS Catalina or macOS Big Sur

[iODBC Driver Manager](http://www.iodbc.org/dataspace/doc/iodbc/wiki/iodbcWiki/Downloads) should be installed before installing the Amazon Timestream ODBC Driver on macOS.

1. Run `AmazonTimestreamODBC-[version].pkg` to install the Amazon Timestream ODBC Driver.
2. [Configure Driver and DSN entries](./docs/user/mac_configure_dsn.md).

### Installing the 32-bit deb file on Ubuntu 20.4 64-bit

The 32-bit dependencies need to be installed before installing the Amazon Timestream ODBC Driver on Ubuntu.

```
sudo apt update
sudo dpkg --add-architecture i386 
sudo apt install unixodbc:i386 unixodbc-dev:i386 odbcinst1debian2:i386 libodbc1:i386 libcurl4-openssl-dev:i386 libssl-dev:i386 uuid-dev:i386 cpp:i386 cpp-9:i386 gcc:i386 g++:i386 zlib1g-dev:i386 linux-headers-$(uname -r) gcc-multilib:i386 g++-multilib:i386 cmake g++-9:i386 gcc-9:i386 gcc-9-multilib:i386 g++-9-multilib:i386 binutils:i386 make:i386
```

You may also want to install [isql32](.\tools) for testing. In the directory where it is installed:
* Type `chmod +x ./isql32` to make it executable.
* Type `./isql32 [DSN]` to make a connection and enter interactive mode.
* Enter SQL statements to run.
* Type `quit` to exit interactive mode.

What needs to be installed to run?
1. Run `AmazonTimestreamODBC_[version]_i386.deb` to install the Amazon Timestream ODBC Driver.
2. [Configure Driver and DSN entries](./docs/user/linux_configure_dsn.md).

### Installing the 64-bit deb file on Ubuntu 20.4
unixODBC should be installed before installing the Amazon Timestream ODBC Driver on Ubuntu.

```
sudo apt update
sudo apt install unixodbc
```

1. Run `AmazonTimestreamODBC_[version]_amd64.deb` to install the Amazon Timestream ODBC Driver.
2. [Configure Driver and DSN entries](./docs/user/linux_configure_dsn.md).

### Installing the 32-bit rpm file on Amazon Linux 2 (TO DO)
To DO - What needs to be installed to run?
1. Run `AmazonTimestreamODBC_[version]_i386.rpm` to install the Amazon Timestream ODBC Driver.
2. [Configure Driver and DSN entries](./docs/user/linux_configure_dsn.md).

### Installing the 64-bit rpm file on Amazon Linux 2
unixODBC should be installed before installing the Amazon Timestream ODBC Driver on Amazon Linux 2.

```
sudo apt update
sudo apt install unixodbc
```
1. Run `AmazonTimestreamODBC_[version]_x86_64.rpm` to install the Amazon Timestream ODBC Driver.
2. [Configure Driver and DSN entries](./docs/user/linux_configure_dsn.md).

## Using the Driver

The driver comes in the form of a library file:
* Windows: `timestreamsqlodbc.dll`
* Mac: `libtimestreamsqlodbc.dylib`
* Linux: `libtimestreamsqlodbc.so`

If using with ODBC compatible BI tools, refer to the tool documentation on configuring a new ODBC driver. In most cases, you will need to make the tool aware of the location of the driver library file and then use it to setup Amazon Timestream database connections.

### Excel
The Amazon Timestream ODBC driver supports both the Windows and macOS versions of Microsoft Excel.

* [Microsoft Excel for Windows](./docs/user/microsoft_excel_support_win.md); or 
* [Microsoft Excel for macOS](./docs/user/microsoft_excel_support_mac.md);

### Connection Strings and Configuring the Driver

A list of options available for configuring driver behaviour is available [here](./docs/user/configuration_options.md).

To setup a connection, the driver uses an ODBC connection string. Connection strings are semicolon-delimited strings specifying the set of options to use for a connection. Typically, a connection string will either:

* specify a Data Source Name containing a pre-configured set of options (`DSN=xxx;`); or
* configure options explicitly using the connection string (`Region=xxx;Auth=xxx;LogLevel=7;...`)

### Troubleshooting

**Illegal parameter value for SQL_AUTOCOMMIT**
By default, the Amazon Timestream ODBC Driver sets `SQL_AUTOCOMMIT=OFF`. pyodbc.connect overrides this and sets the value to `ON`.

To resolve the issue, excitly set the value `pyodbc.connect(connstr,autocommit=True)`

## Building from Source

### Building

Please refer to the [build instructions](./docs/dev/BUILD_INSTRUCTIONS.md) for detailed build instructions on your platform.

### Testing

Please refer to the [test instructions](./docs/dev/run_tests.md) for instructions on running the automated tests for your platform.

## Licensing

See the [LICENSE](./LICENSE) file for our project's licensing. We will ask you to confirm the licensing of your contribution.

## Copyright

Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
