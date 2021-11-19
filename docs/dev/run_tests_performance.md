# Running the Performance Tests

## Preparation

* [Python 3](https://www.python.org/downloads/) is required to run the performance tests. This should also include pip3.
* Setup a database named `ODBCPerfTest` with the table `DevOps`. More than 10,000 should be added using the [continuous ingestor tool](https://github.com/awslabs/amazon-Database-tools/tree/mainline/tools/continuous-ingestor). Once the data has been ingested into the `DevOps` table, these tests use static queries and do not need to have live data. 
* Setup a database named `perfdb_hcltps` with the table `perftable_hcltps`. Data should be ingested into this database for more than one hour using the [continuous ingestor tool](https://github.com/awslabs/amazon-Database-tools/tree/mainline/tools/continuous-ingestor). This need to be run continuously while running these tests, as they are time sensitive and are based on the current time and the current time - 1 hour.

## Running the Automated Tests

1. By default, the performance tests run 10 iterations, to change the number of iterations, modify the value for ITERATION_COUNT in [performance_odbc_results.cpp](../../src/PerformanceTests/PTODBCResults/performance_odbc_results.cpp). 
2. By default the queries being run against `ODBCPerfTest.DevOps` return 10,000 record. To change the query; including the number of records returned, modify the m_query variable. 
3. Follow the [build instructions](./BUILD_INSTRUCTIONS.md) to compile the performance tests executable.
4. Using Developer Powershell in Windows, from the project directory, run `run_test_runner.bat`.
5. The results will be printed out in the command line and the file `test_output.html` will be generated.

**Notes:** 
* The performance tests have only been run on Windows Server 2019 in AWS EC2 instances. We recommend running the performance tests in an AWS EC2 instance in same region as the the database for consistency.
* The performance tests are not separated between the static queries and the time based queries. To run only one set of performance tests, the unwanted tests should be commented out and the test suite should be recompiled.
