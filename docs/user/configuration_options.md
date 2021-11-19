# Configuration Options

## Driver Specific Options
The driver specific options can be set in the `odbcinst.ini` file for macOS or Linux, or in the `Drivers` tab in `ODBC Data Source Administrator` for Windows. 

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `Driver` | Driver name.| string | databaseodbc |
| `LogLevel` | Severity level for driver logs. | integer<br />one of `0`(OFF), `1`(FATAL), `2`(ERROR), `3`(WARNING), `4`(INFO), `5`(DEBUG), `6`(TRACE), `7`(ALL) | `0`(OFF) |
| `LogOutput` | Location for storing driver logs. | string | WIN: `TEMP environment variable`, MAC/Linux: `/tmp` |

## DSN Specific Options
The DSN specific options can be set in the `odbc.ini` file for macOS or Linux, or in the `User DSN` /`System DSN` tab in `ODBC Data Source Administrator` for Windows. 

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `Driver` | Driver name.| string | Databaseodbc |
| `DSN` | **D**ata **S**ource **N**ame used for configuring the connection. | string | `<NONE>` |
| `Auth` | Authentication mode. | one of `DEFAULT` | `DEFAULT`
| `LogLevel` | Severity level for driver logs. | integer<br />one of `0`(OFF), `1`(FATAL), `2`(ERROR), `3`(WARNING), `4`(INFO), `5`(DEBUG), `6`(TRACE), `7`(ALL) | `0`(OFF) | `0`(OFF) |
| `LogOutput` | Location for storing driver logs. | string | WIN: `TEMP environment variable`, MAC/Linux: `/tmp` |

**Note:** We recommend setting the LogOutput for the driver (in the `odbcinst.ini` file or registry settings) and not the DSN (in the `odbc.ini` file or registry settings). Otherwise the first log file will live in a temporary folder and the DSN specified LogOutput value will not take effect until the next time the driver is initialized.

#### Database Default Authentication Options

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `ProfileName` | Profile name for the AWS credentials. <NONE> means loading default credential chain. | string | `<NONE>` |

#### Database Default Authentication Options

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `UID` | The AWS user access key id.| string | `<NONE>` |
| `PWD` | The AWS user secret access key. | string | `<NONE>` |

#### AWS SDK Options

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `RequestTimeout` | The time in milliseconds the AWS SDK will wait for a query request before timing out. Non-positive value disables request timeout. | int | `3000` |
| `ConnectionTimeout` | Socket connect timeout. Value must be non-negative. A value of 0 disables socket timeout. | int | `1000` |
| `MaxRetryCountClient` | The maximum number of retry attempts for retryable errors with 5XX error codes in the SDK. The value must be non-negative, 0 means no retry. | int | `<NONE>` |
| `MaxConnections` | The maximum number of allowed concurrently opened HTTP connections to the Database service. The value must be positive. | int | `25` |

#### Endpoint Configuration Options

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `EndpointOverride` | The endpoint override for the Database service. It overrides the region. It is an advanced option. | string | `<NONE>` |

**NOTE:** Administrative privileges are required to change the value of logging options on Windows / macOS.
