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

You can use the installers generated as part of the most recent release.

### Windows

1. Run the `.msi` installer to install the Amazon Timestream ODBC Driver.
2. [Test the connection](./docs/user/windows_configure_dsn.md) using ODBC Data Source Administrator.

### Mac

iODBC Driver Manager should be installed before installing the Amazon Timestream ODBC Driver on Mac.

1. Run the `.pkg` installer to install the Amazon Timestream ODBC Driver.
2. Configure a Driver and DSN entry for the Amazon Timestream ODBC Driver, following the instructions [here](./docs/user/mac_configure_dsn.md).

### Linux `.deb` - TO DO

### Linux `.rpm - TO DO

## Using the Driver

The driver comes in the form of a library file:
* Windows: `timestreamsqlodbc.dll`
* Mac: `libtimestreamsqlodbc.dylib`

If using with ODBC compatible BI tools, refer to the tool documentation on configuring a new ODBC driver. In most cases, you will need to make the tool aware of the location of the driver library file and then use it to setup Amazon Timestream database connections.

### Connection Strings and Configuring the Driver

A list of options available for configuring driver behaviour is available [here](./docs/user/configuration_options.md).

To setup a connection, the driver uses an ODBC connection string. Connection strings are semicolon-delimited strings specifying the set of options to use for a connection. Typically, a connection string will either:

1. specify a Data Source Name containing a pre-configured set of options (`DSN=xxx;`)
2. or configure options explicitly using the string (`Region=xxx;Auth=xxx;LogLevel=7;...`)

## Building from source

### Building

Please refer to the [build instructions](./docs/dev/BUILD_INSTRUCTIONS.md) for detailed build instructions on your platform.

### Testing

**NOTE**: Some tests in ITODBCConnection will fail if a test DSN (Data Source Name) is not configured on your system. Refer to "Running Tests" in the [build instructions](./BUILD_INSTRUCTIONS.md) for more information on configuring this.

## Licensing

See the [LICENSE](./LICENSE) file for our project's licensing. We will ask you to confirm the licensing of your contribution.

## Copyright

Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
