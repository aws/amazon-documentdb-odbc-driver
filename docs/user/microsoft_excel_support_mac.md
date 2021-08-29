#  macOS - Connecting to Micosoft Excel

## Prerequisites

* Microsoft Excel 2016 and higher
* [Amazon Timestream](https://aws.amazon.com/timestream/)
* Amazon Timestream ODBC driver
* A preconfigured [User or System DSN](mac_configure_dsn.md)

## Loading Data 

* Open a blank workbook in **Microsoft Excel**.
* Click on **Data** > **Get Data** > **From Database**.
* From the **User DSN** or **System DSN** tab, select the desired data source and click **OK**.
* Enter the credentials if required and click on **OK**.
* Select a table from the list. 
* Edit the SQL statement if required and click **Run**:
    * Remove the semi colon from the generated query
    * You may want to add a limit clause to restrict the number of records being returned. For example:
    <pre>SELECT * FROM ODBCTest.DevOps LIMIT 10000</pre>
* Click on **Return Data**. Select the sheet sheet and click **OK**.

The data will be loaded in the spreadsheet.

## Refreshing Data

To refresh the data click on **Table** > **Refresh** or on **Data** > **Refresh**.

## Exporting as CSV

* Click on **File** > **Save As**.
* Type the file name.
* Select the location where you want to save the file.
* Set the **File Format** to `CSV UTF-8 (Comma delimited) (*.csv)`.
* Click **Save**.

The data will be exported to selected location in CSV format.

## Troubleshooting

* You may need to remove `;` from SQL statement to load data preview.
* **AWS Profile** authentication is not supported in Excel for Mac. We recommend using IAM authentication and populating the AWS Access Key ID, AWS Secret Access Key and AWS SessionToken in the `odbc.ini` file from the values in the `~/.aws/credentials` file.
