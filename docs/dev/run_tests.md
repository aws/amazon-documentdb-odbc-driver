# Database ODBC Driver Automated Tests

## Preparation

### Loading Test Datasets

Loading a dataset requires an [Database](database_link_placeholder) service. If missing, please refer to the documentation on how to set them up.

In order for all of the tests to pass, [create two sample databases](database_link_database_getting_started) `sampleDB` and `ODBCTest`. If more databases exist for the region, the catalog tests for the region will fail and should be disabled.

```
./build/odbc/bin/tests --gtest_filter=-'TestSQLTables.*'
```

## Running Tests
Refer to the documentation for your operating system:
* [Windows](./run_tests_win.md)
* [macOS](./run_tests_mac.md)
* [Linux](./run_tests_linux.md)
* [Performance](./run_tests_performance.md)
