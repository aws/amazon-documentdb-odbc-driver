# Building the Amazon Timestream Power BI Connector

The connector file `AmazonTimestream.mez` is a ZIP archive containing the power query files.

## Prerequisites
1. Install Visual Studio 2019.
2. Install the [Power Query SDK](https://marketplace.visualstudio.com/items?itemName=Dakahn.PowerQuerySDK).
3. Setup AWS IAM Credentials or AWS Profile authentication
4. An Amazon Timestream sample database called `PowerBI` containing the `DevOps` table.

## Running the Tests
1. Load the `src\PowerBIConnector\AmazonTimestreamConnector\AmazonTimestreamConnector.mproj` file from the project directory.
    * Visual Studio may complain about the project not being able to load properly, right click on the solution and reload the project. This is related to tooling in the Power Query SDK.
2. Edit the AmazonTimestreamConnector.query.pq file and set `Region` to the region where the `PowerBI` database is located.
    * If you are using `AWS Profile` authentication and not using `default`, enter the `Profile Name`.
    * If you are using `AWS IAM Credentials` authentication and multi-factor authentication, enter the `AWS IAM Session Token`.
3. Run the project. The first time, you will be asked to enter your credential information. Select `Anonymous` for `AWS Profile` authentication, or `UsernamePassword` for `AWS IAM Credentials` authentication. Click `Set Credential` and close the `M Query Output` window. 

   **Note:** The tests are data dependent and while the sample databases created have the same schema, the dates and data values will be different. The tests for aggregate functions like sum and the minimum time, and mathmatical functions like sin and tangent may have to be updated.

4. Run the project to view the test results.

## Rebuilding the Connector
* Run `.\AmazonTimestreamConnector.ps1` from within the `src\PowerBIConnector` directory. The `AmazonTimestreamConnector.mez` file will be created which can then be copied to the `<User>\Documents\Power BI Desktop\Custom Connectors` directory (or to `<User>\OneDrive\Documents\Power BI Desktop\Custom Connectors` if using OneDrive). To use the connector with the `On-premises data gateway`, it should also be copied to `C:\Windows\ServiceProfiles\PBIEgwService\Documents`.
