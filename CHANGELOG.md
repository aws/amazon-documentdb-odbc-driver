# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
- Changed `AWS PROFILE` authentication mode to `AWS_PROFILE`
- Changed default log level from WARNING to OFF
- Removed the default log location for Windows

## [v0.1.0](https://github.com/Bit-Quill/timestream-odbc/releases/tag/v0.1.0) - 2021-02-12
### Added
- The ability to connect to Amazon Timestream using IAM and AWS Profile.
- The ability to retrieve row number from a Select query, but no support for any SQL data types yet.
### Fixed
- Memory leak in logging and query.

