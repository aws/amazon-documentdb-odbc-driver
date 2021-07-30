All files are available in '/Library/ODBC/timestream-odbc' after installation.

To setup a connection, you can use DSN to store your data source connection information,
1. Open 'iODBC Data Source Administrator'.
2. Go to 'User DSN'.
3. Select 'AMAZON TIMESTREAM ODBC DSN' and click on 'Configure'.
4. Update the connection string values. For the list of all supported options, check '/Library/ODBC/timestream-odbc/doc/README.md'.
5. Click 'Ok' to save changes.

If using with ODBC compatible BI tools, refer to the tool documentation on configuring a new ODBC driver. The typical requirement is to make the tool aware of the location of the driver library file and then use it to setup database connections.

For example, if you want to use Microsoft Power BI with Amazon Timestream,
1. Open 'Power BI Desktop'.
2. Select 'Get Data'.
3. Select 'ODBC'.
4. Select the appropriate item from the DSN list (e.g. 'Amazon Timestream ODBC DSN').
5. Optionally enter a SQL Statement (you may want to limit large result sets).
6. Click 'OK'
7. Enter the connection information and Click on 'Connect'. All connection attributes will be retrived.
