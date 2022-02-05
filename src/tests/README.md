# performance-test-odbc
Performance test framework for testing ODBC driver

# How to run with executable (executable is built with main build scripts)
1. Setup test database system under a dsn
2. Specify test plan input file
3. Specify output file 
4. Run executable and pass in command line arguments: dsn-name input-file output-file
e.g. `performance.exe "documentdb-perf-test" "Performance_Test_Plan.csv" "Performance_Test_Results.csv"`
e.g. output console:
```
%%__PARSE__SYNC__START__%%
%%__QUERY__%% SELECT * FROM performance.employer limit 1
%%__CASE__%% 1: Rows retrieved 1
%%__STATUS__%% Success
%%__MIN__%% 62 ms
%%__MAX__%% 81 ms
%%__MEAN__%% 67 ms
%%__MEDIAN__%% 66 ms
%%__PARSE__SYNC__END__%%
SQLExecDirect->SQLBindCol->SQLFetch Time dump: 62 ms, 64 ms, 64 ms, 65 ms, 66 ms, 66 ms, 67 ms, 68 ms, 68 ms, 81 ms
```

5. If no command line arguments are passed, by default:
   1. input-file = `build\odbc\cmake\tests\performance\Performance_Test_Plan.csv`
   2. output-file = `build\odbc\bin\Debug\Performance_Test_Results.csv`
   3. dsn-name = `documentdb-perf-test`
6. Instead of using command line, you can open executable file in Visual Studio and run executable in debug mode
   1. Executable is located at `build\odbc\bin\Debug`

# Test Plan CSV File
Following headers are required:
1. query : string (can contain commas and newlines)
2. limit : integer > 0
3. test_name : string (can contain commas and newlines)
4. loop_count : integer > 0
5. skip_test : TRUE or FALSE