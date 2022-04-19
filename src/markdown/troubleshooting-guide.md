# Troubleshooting Guide

- [Logs](#logs)

## Logs

When troubleshooting, it can be helpful to view the logs so that you might be able 
to resolve the issue on your own or at least have more context to provide when seeking support. 
On Windows, the driver's logs will be written to `%TEMP%/documentdb_odbc.log` by default.
On Mac, the default log path is `~/Library/Logs/documentdb_odbc.log`. 
On Linux/Unix, the default log path is `~/var/log/documentdb_odbc.log`.
On Windows, you may change the default path in the DSN configuration window.
In any platform, you may pass your log path / log level in the connection string.
The keyword for log path is `log_path` and the keyword for log level is `log_level`. 

### Setting Logging Level and Location
There are the following levels of logging:

| Property Value | Description |
|--------|-------------|
| `ERROR` | Shows messages classified as ERROR.|
| `INFO` | Shows messages classified as INFO and ERROR.|
| `DEBUG` | Shows messages classified as DEBUG, INFO and ERROR.|
| `OFF` | No log messages displayed.|

| Property Name | Description | Platform | Default |
|--------|-------------|--------|---------------|
| `log_level` | The log level for all sources/appenders. | All Platforms | `ERROR` |
| `log_path` | The location for file logging. | Windows | `%TEMP%/documentdb_odbc.log` |
| `log_path` | The location for file logging. | MacOS | `~/Library/Logs/documentdb_odbc.log` |
| `log_path` | The location for file logging. | Linux/Unix | `~/var/log/documentdb_odbc.log` |

To set these properties, use the connection string with the following format 
`<property-name>=<property-value>`. 

For example: (Note: The capitalization does not matter.)
- In Windows, append `LOG_PATH="C:\Users\Name\Desktop\DocumentDB ODBC Driver\odbc_log_test.txt";LOG_LEVEL=DEBUG;` 
to your connection string.
    * You can also set the log path and log level from the configuration window in the Microsoft ODBC Administrator. 
    * Click on the drop menu for setting the log level
    * Enter the desired log file path in the field next to the label `Log File`. The user needs to ensure that the directory mentioned in the log file path does exist, or ODBC will be unable to create the log file. Note that since ODBC Driver will append to the log file, it is the user's responsibility to ensure that the file at the log file path that they provide is clean or does not exist so ODBC driver will create a new one. 

- In MacOS/Linux/Unix, append `LOG_PATH="~/odbc_log_test.log";LOG_LEVEL=ERROR;` to your connection string, or append
`LOG_PATH` and `LOG_LEVEL` as keywords in the ODBC manager. 
- If you just want to change the log level, append `LOG_LEVEL=<desired-log-level>;` to your connection string.