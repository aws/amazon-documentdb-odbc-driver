# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [v0.4.0](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.4.0) - 2021-07-07
### Added
- Linux 32-bit and Linux 64-bit support for the ODBC Driver
- Linux support for the ODBC sample client application 

### Fixed
- Excessive memory usage
- Improved test cases
- Removed "ES_" and "TS_" from function and variable names
- All warnings in driver and enabled -Werror for Mac and Linux build

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

