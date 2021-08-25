#  Connecting AWS Timestream to Microsoft Excel on Windows

## Prerequisites

* Microsoft Excel 2016 and higher
* [Amazon Timestream](https://aws.amazon.com/timestream/)
* Amazon Timestream ODBC driver
* A preconfigured [User or System DSN](../../README.md)

## Loading Data 

* Open a blank workbook in Microsoft Excel.
* Click on **Data** > **Get Data** > **From Other Sources** > **From ODBC**
* Select the desired data source. For improved performance you may want to enter a query directly in **Advanced options** and limit the results returned. For example `SELECT * FROM sampleDB.IoT`. Click **OK**.
* For `AWS IAM Credentials`, `Identity Provider: Azure AD` or `Identity Provider: Okta connetions`, select **Database** from the left menu and enter a **User name** and **Password**. For AWS Profile connections, Select **Default or Custom** from the left menu. Click **Connect**.
* If you did not enter a query, select a table from list to load data preview; otherwise the preview will display automatically. Click **Load**.
* The data will be loaded in the spreadsheet.

## Refreshing Data

To refresh the data click on **Query** > **Refresh** or **Data** > **Refresh**.

## Exporting as CSV

* Click on **File** > **Save As**.
* Select the location where you want to save the file.
* Type the file name.
* Select the type **CSV UTF-8 (Comma delimited) (\*.csv)**.
* Click **Save**.

The data will be exported to the selected location in CSV format.

## Troubleshooting

* There is a potential performance issue when loading large amounts of data. We recommend using **Advanced options** to enter the query statement explicitly to load the data. For example, `SELECT * FROM sampleDB.IoT`.
* If there are a lot of rows in the table, Microsoft Excel will process the entire table even if exceeds Excel's limit of a million records. You may want to use Advanced options to limit the query results. For example, `SELECT * FROM sampleDB.IoT LIMIT 10000`.
* Your Data Source Settings are saved once you connect. To modify them, select **Data** > **Get Data** > **Data Source Settings...**.
