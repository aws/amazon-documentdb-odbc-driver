# Amazon DocumentDB ODBC Driver Setup

## Pre-requisites
1. Install JAVA JDK https://corretto.aws/downloads/latest/amazon-corretto-17-x64-windows-jdk.msi
2. Make sure that JAVA_HOME envrionment variable is set and %JAVA_HOME%\bin is part of the PATH environment variable

## Install DocumentDB ODBC Driver
1. Download the [DocumentDB ODBC msi](https://github.com/aws/amazon-documentdb-odbc-driver/releases).
2. Double-click the installer.
3. Follow the instructions on the installer window and finish the installation.

## Notes
Microsoft Visual C++ Redistributable Package will be installed automatically if you do not already have it installed on your system.

When uninstalling, the Microsoft Visual C++ Redistributable Package will not be uninstalled.

## Next Steps

- [DSN](dsn-configuration.md)
