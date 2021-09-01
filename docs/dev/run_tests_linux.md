# Linux - Running the Amazon Timestream ODBC Driver Automated Tests

## Preparation

See [Running Amazon Timestream Automated Tests](./run_tests.md) for information on how to load the test datasets.

## Running the Automated Tests

**NOTES:**

* The automated tests cannot be run for the 32-bit version of the Timestream ODBC Driver for Amazon Linux 2. To generate the .rpm file, a Docker image must be used which does not support the codecvt import.
* Test DSNs named `timestream-aws-profile`,  `timestream-iam`,  `timestream-aad` and `timestream-okta` must be set up in order for certain tests to pass. If you do not have Azure AD and Okta setup, exclude the `TestSQLConnectSAMLAuth` and `TestSQLDriverConnectSAMLAuth` tests.
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
    * For `[timestream-iam]`, populate `UID` and `PWD`.
    * For `[timestream-aad]`, populate `AADApplicationID`, `AADClientSecret`, `RoleARN`, `AADTenant`, `IdpARN`, `IdpUserName` and `IdpPassword`.
    * For `[timestream-okta]`, populate `IdpHost`, `OktaApplicationID`, `RoleARN`, `IdpARN`, `IdpUserName` and `IdpPassword`.
3. Using `iODBC Administrator`, test the connections.

**Note:** You do not need to populate the `[timestream-aad]` or `[timestream-okta]` sections if your database does not have Azure AD or Okta setup and you are skipping the tests.

### Running Tests from Unix Shell
Tests can be executed from the Unix shell. To run all tests, from the project root directory (replacing the values), run:

<pre>
export AAD_CONNECT_STRING="<i>[AAD_CONNECT_STRING]</i>"
export OKTA_CONNECT_STRING="<i>[OKTA_CONNECT_STRING]</i>"

./build/odbc/bin/tests
</pre>

**Note:** The AAD_CONNECT_STRING and OKTA_CONNECT_STRING entries should be separated by a ";" without spaces. For example:

<pre>
export AAD_CONNECT_STRING="Driver=Amazon Timestream ODBC Driver;Region=us-east-1;Auth=AAD;IdpName=AzureAD;AADApplicationID=<i>xxx</i>;AADClientSecret=<i>xxx</i>;RoleARN=<i>xxx</i>;AADTenant=<i>xxx</i>;IdpARN=<i>xxx</i>;IdpUserName=<i>xxx</i>;IdpPassword=<i>xxx</i>"
</pre>

To exclude the Okta and Azure Active Directory tests, run:

<pre>
./build/odbc/bin/tests --gtest_filter=-'TestSQLConnectSAMLAuth.*':'TestSQLDriverConnectSAMLAuth.*'
</pre>

To exclude the table catalog tests, run:

<pre>
./build/odbc/bin/tests --gtest_filter=-'TestSQLTables.*'
</pre>
