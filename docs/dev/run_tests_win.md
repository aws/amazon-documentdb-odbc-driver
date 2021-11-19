# Windows - Running the Database ODBC Driver Automated Tests

## Preparation

See [Running Database Automated Tests](./run_tests.md) for information on how to load the test datasets.

## Running the Automated Tests

Tests can be executed directly using **Visual Studio** or through **Developer Powershell**. See [Configuration Options](../user/configuration_options.md) for more details on specific configuration options.

**NOTES:**

* If your region contains more databases other than `sampleDB` and `ODBCTest`, the table catalog tests should be disabled.
```
./build/odbc/bin/tests --gtest_filter=-'TestSQLTables.*'
```

### Windows database-default setup.
1. Run `ODBC Data Sources (64-bit)`.
2. On the `User DSN` or `System DSN` tab, click `Add...`.
3. Select `Database ODBC Driver` and click `Finish`.
4. Enter `database-default` in the `Data Source Name` field. By default, `Auth` should be set to `Default`.
5. Click `Test`. You should get a connection successful message. Click `OK` to close the message box.
6. Click `OK` to close the setup dialog.


### Running Tests from Visual Studio

Tests can be executed directly using **Visual Studio** by setting the desired test as a **Start up Project**

>* **Right click** the **tests** project in the **Solution Explorer**
>* Select **Set as Startup Project**
>* Run the test by selecting **Local Windows Debugger** in the toolbar at the top of the application
>* By default all tests are run. If you do not have Okta or Azure AD setup the TestSQLConnectSAMLAuth. and TestSQLDriverConnectSAMLAuth tests will fail.

### Running Tests from Developer Powershell

Tests can be executed from Developer Powershell. To run all tests, set the AAD_CONNECT_STRING and OKTA_CONNECT_STRING environment variables; then from the project root directory, run:

<pre>
.\build\odbc\bin\Release\tests
</pre>

To exclude the table catalog tests, run:

<pre>
.\build\odbc\bin\tests --gtest_filter=-'TestSQLTables.*'
</pre>
