# Windows - Running the Amazon Timestream ODBC Driver Automated Tests

## Preparation

See [Running Amazon Timestream Automated Tests](https://github.com/Bit-Quill/timestream-odbc/blob/update_docs/docs/dev/run_tests.md) for information on how to load the test datasets.

## Running the Automated Tests

Tests can be executed directly using **Visual Studio** or through **Developer Powershell**. See [Configuration Options](../user/configuration_options.md) for more details on specific configuration options.

**NOTES:**

* Test DSNs named `timestream-aws-profile`,  `timestream-iam`,  `timestream-aad` and `timestream-okta` must be set up in order for certain tests to pass. If you do not have Azure AD and Okta setup, exclude the `TestSQLConnectSAMLAuth` and `TestSQLDriverConnectSAMLAuth` tests.

### Windows timestream-aws-profile setup.
1. Run `ODBC Data Sources (64-bit)`.
2. On the `User DSN` or `System DSN` tab, click `Add...`.
3. Select `Amazon Timestream ODBC Driver` and click `Finish`.
4. Enter `timestream-aws-profile` in the `Data Source Name` field. By default, `Auth` should be set to `AWS Profile` and the `Region` to `us-east-1`.
5. Click `Test`. You should get a connection successful message. Click `OK` to close the message box.
6. Click `OK` to close the setup dialog.

### Windows timestream-iam setup.
1. Run `ODBC Data Sources (64-bit)`.
2. On the `User DSN` tab, click `Add...`.
3. Select `Amazon Timestream ODBC Driver` and click `Finish`.
4. Enter `timestream-iam` in the `Data Source Name` field.
5. Select `AWS IAM Credentials` in the `Auth` drop-down.
6. Enter the `Access Key ID`, `Secret Access Key` and `Session Token`. By default, the `Region` to `us-east-1`.
5. Click `Test`. You should get a connection successful message. Click `OK` to close the message box.
6. Click `OK` to close the setup dialog.

### Windows timestream-aad setup.
1. Run `ODBC Data Sources (64-bit)`.
2. On the `User DSN` tab, click `Add...`.
3. Select `Amazon Timestream ODBC Driver` and click `Finish`.
4. Enter `timestream-aad` in the `Data Source Name` field.
5. Select `Identity Provider: Azure AD` in the `Auth` drop-down.
6. Enter the `Role ARN`, `IdP Name`, `IdP Username`, `IdP Password`, `IdP ARN`, `AAD Application ID`, `AAD Client Secret` and `AADTenant`. By default, the `Region` to `us-east-1`.
5. Click `Test`. You should get a connection successful message. Click `OK` to close the message box.
6. Click `OK` to close the setup dialog.

### Windows timestream-okta setup.
1. Run `ODBC Data Sources (64-bit)`.
2. On the `User DSN` tab, click `Add...`.
3. Select `Amazon Timestream ODBC Driver` and click `Finish`.
4. Enter `timestream-okta` in the `Data Source Name` field.
5. Select `AWS IAM Credentials` in the `Auth` drop-down.
6. Enter the `Role ARN`, `IdP Name`, `IdP Username`, `IdP Password`, `IdP ARN`, `IdP Host` and `Okta Application ID`. By default, the `Region` to `us-east-1`.
5. Click `Test`. You should get a connection successful message. Click `OK` to close the message box.
6. Click `OK` to close the setup dialog.

### Set the Environment Variables

In order to successfully run the Okta and Azure AD tests, the AAD_CONNECT_STRING and OKTA_CONNECT_STRING environment variables need to be set. Using Developer Powershell, run the following (replacing the values):

<pre>
set AAD_CONNECT_STRING="<i>[AAD_CONNECT_STRING]</i>"
set OKTA_CONNECT_STRING="<i>[OKTA_CONNECT_STRING]</i>"
</pre>

**Note:** The AAD_CONNECT_STRING and OKTA_CONNECT_STRING entries should be separated by a ";" without spaces. For example:

<pre>
set AAD_CONNECT_STRING="Driver=Amazon Timestream ODBC Driver;Region=us-east-1;Auth=AAD;IdpName=AzureAD;AADApplicationID=<i>xxx</i>;AADClientSecret=<i>xxx</i>;RoleARN=<i>xxx</i>;AADTenant=<i>xxx</i>;IdpARN=<i>xxx</i>;IdpUserName=<i>xxx</i>;IdpPassword=<i>xxx</i>"
</pre>

**NOTE:** Using `set` will only work for the current terminal session. You can also set the environment variables permanently through `Advanced Settings` in `Control Panel`.

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

To exclude the Okta and Azure Active Directory tests, run:

<pre>
.\build\odbc\bin\Release\tests --gtest_filter=-'TestSQLConnectSAMLAuth.*':'TestSQLDriverConnectSAMLAuth.*'
</pre>
