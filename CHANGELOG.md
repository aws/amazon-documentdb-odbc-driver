# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [v1.0.2](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v1.0.2) - 2021-09-17
### Fixed
- Okta authentication not working in 32-bit Amazon Linux 2.

## [v1.0.1](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v1.0.1) - 2021-09-14
### Fixed
- Obfuscated the session token for IAM authentication in the ODBC user interface for Windows and make sure it is not in the logs.

## [v1.0.0](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v1.0.0) - 2021-09-01
### Added
- 32 bit support for Amazon Linux 2.

## [v0.5.2](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.5.2) - 2021-08-29
### Added
- Sample odbc.ini and odbcinst.ini files.
- Convert region to lower case when connecting.

### Fixed
- Don't treat trailing 0s as truncation during data conversion.
- Seg fault in TestSQLGetCursorName when no AWS credentials are supplied.
- Use exact number for longs in data-conversion.
- Driver version is now displayed in iODBC Administrator.

## [v0.5.1](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.5.1) - 2021-08-17
### Added
- Big Sur support.
- Timestamps in logs.

### Fixed
- Invalid fieldnames when UserID/Password is empty in ODBC Administrator for Windows.
- Infinite loop in result cursor logic.
- Define DBMS_NAME for Linux and macOS.
- Simplified .deb and .rpm filenames.
- NPE in TestSQLGetCursorName.
- Updated default path for Windows logging.
- Updated AWS SDK version to 1.9.79.

## [v0.5.0](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.5.0) - 2021-08-04
### Added
- More connection tests.

### Fixed
- Removed the version and bitness from the default path in the Windows ODBC installers.
- Set the default log level to "OFF" instead of "WARNING".
- Updated the DSN setup documentation for Windows.
- AWS AppSec Finding: Added nullptr checks before dereferencing.
- Tab order in ODBC UI for Windows.
- Hid fields in the ODBC UI for Windows instead of disabling them.
- Automated SQLCancel tests.

## [v0.4.3](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.4.3) - 2021-07-29
### Added
- Merged tests into one executable
- Integration tests for data conversion
- Code coverage
- Performance test framework

### Fixed
- Removed the checks for ordinal position in SQLColumns tests
- AWS AppSec Finding: Memory out of bound error in makeConnectString

## [v0.4.2](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.4.2) - 2021-07-20
### Fixed
- Renamed odfesqlodbc directory to timestreamsqlodbc and updated references
- Renamed odfeenlist directory to timestreamenlist and update references
- AWS AppSec Finding: Unsafe C functions that do not check memory bounds

## [v0.4.1](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.4.1) - 2021-07-08
### Fixed
- Intermittent crash in Excel in multi-connection environments on disconnect. Aws::InitApi and Aws::ShutdownApi is now only called once per application instead of once per connection.

## [v0.4.0](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.4.0) - 2021-07-07
### Added
- Linux 32-bit and Linux 64-bit support for the ODBC Driver
- Linux support for the ODBC sample client application 

### Fixed
- Excessive memory usage
- Improved test cases
- Removed "ES_" and "TS_" from function and variable names
- All warnings in driver and enabled -Werror for Mac and Linux build
- Plus sign (+) not working in connection string
- Removed "es_" and "ts_" from filename prefixes
- Failing tests when using Okta or Azure AD authentication

## [v0.3.2](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.3.2) - 2021-06-30
### Fixed
- Password appearing in plain text
- Plus sign not working in connection string values
- Possible malformed AzureAD/Okta request body

## [v0.3.0](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.3.0) - 2021-06-15
### Added
- The ability to connect using Azure Active Directory and Okta.
- Support for SQL Metadata (SQLColumns, SQLTables, SQLGetInfo, SQLGetTypeInfo)
- Query Cancellation
- Support for Excel and PowerBI
### Changed
- Removed "ES_" and "TS_" from filenames
- Changed the user interface in Windows to mask the passwords and secret keys
- Paginated the results
- Changed SQLPrepare to get metadata rather than running the query

## [v0.2.0](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.2.0) - 2021-03-26
### Added
- The ability to query data from Amazon Timestream.
- Full support for Amazon Timestream supported datatype: int, bigint, boolean, double, varchar, array[T,...], row(T,...), date, time, timestamp, interval, timeseries[row(timestamp,T,...)], unknown
- Support for SQL query execution (SQLPrepare, SQLExecute, SQLExecDirect)
- Support for SQL data type conversion (SQLGetData, SQLBindCol)
- Support for SQL results (SQLFetch, SQLMoreResults, SQLNumResultCols)
- Support for SQL column descriptors (SQLDescribe, SQLGetDescRec, SQLSetDescRec, SQLGetDescField, SQLSetDescField)
- Support `ProfileName` in `AWS_PROFILE` authentication mode
### Changed
- `AWS PROFILE` authentication mode to `AWS_PROFILE`
- Default log level from WARNING to OFF
- Removed the default log location for Windows

## [v0.1.0](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.1.0) - 2021-02-12
### Added
- The ability to connect to Amazon Timestream using IAM and AWS Profile.
- The ability to retrieve row number from a Select query, but no support for any SQL data types yet.
### Fixed
- Memory leak in logging and query.

