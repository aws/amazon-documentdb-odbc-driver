# performance-test-odbc
Performance test framework for testing ODBC driver

# How to run with executable
1. Setup test database system under a dsn
1. Specify test plan input file
1. Specify output file 
1. Run executable and pass in command line arguments: dsn-name input-file output-file
e.g. `performance.exe "documentdb-perf-test" "Performance_Test_Plan.csv" "Performance_Test_Results.csv"`

# Test Plan CSV File
Following headers are required:
1. query
1. limit
1. test_name
1. loop_count
1. skip_test = TRUE or FALSE

# Building project on Windows
1. Open project in Visual Studio 16 2019
1. Run CMake configure
1. Run CMake command: `cmake -S . -B out\build\ -G "Visual Studio 16 2019"`
1. Build project (F7) 
1. If no command line arguments are passed, by default:
   1. input-file = build\odbc\cmake\PerformanceTests\performance\Performance_Test_Plan.csv
   1. output-file = build\odbc\cmake\PerformanceTests\performance\Performance_Test_Results.csv
   1. dsn-name = documentdb-perf-test