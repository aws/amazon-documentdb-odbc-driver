# Linux - Running the Database ODBC Driver Automated Tests

## Preparation

See [Running Database Automated Tests](./run_tests.md) for information on how to load the test datasets.

## Running the Automated Tests

**NOTES:**

* The automated tests cannot be run for the 32-bit version of the Database ODBC Driver for Amazon Linux 2. To generate the .rpm file, a Docker image must be used which does not support the codecvt import.
* If your region contains more databases other than `sampleDB` and `ODBCTest`, the table catalog tests should be disabled.
```
./build/odbc/bin/tests --gtest_filter=-'TestSQLTables.*'
```

### Setup the DSN Entries

1. Open [src/Tests/Tests/odbcinst-linux64.ini](../../src/Tests/Tests/odbcinst-linux64.ini) or [src/Tests/Tests/odbcinst-linux32.ini](../../src/Tests/Tests/odbcinst-linux32.ini) and manually add the entries to `/Library/ODBC/odbcinst.ini`.
2. Open [src/Tests/Tests/odbc-linux64.ini](../../src/Tests/Tests/odbc-linux64.ini) or [src/Tests/Tests/odbc-linux32.ini](../../src/Tests/Tests/odbc-linux32.ini)
 * To create a System DSN, manually add the entries to `/Library/ODBC/odbc.ini`. 
 * To create a User DSN, manually add the entries to `~/Library/ODBC/odbc.ini`.
 * In the odbc.ini file populate the following settings (See [Configuration Options](../user/configuration_options.md) for more details).
3. Using `iODBC Administrator`, test the connections.

### Running Tests from Unix Shell
Tests can be executed from the Unix shell. To run all tests, from the project root directory (replacing the values), run:

<pre>

./build/odbc/bin/tests
</pre>

<pre>
export AAD_CONNECT_STRING="Driver=Database ODBC Driver;Region=us-east-1;Auth=AAD;IdpName=AzureAD;AADApplicationID=<i>xxx</i>;AADClientSecret=<i>xxx</i>;RoleARN=<i>xxx</i>;AADTenant=<i>xxx</i>;IdpARN=<i>xxx</i>;IdpUserName=<i>xxx</i>;IdpPassword=<i>xxx</i>"
</pre>

To exclude the table catalog tests, run:

<pre>
./build/odbc/bin/tests --gtest_filter=-'TestSQLTables.*'
</pre>
