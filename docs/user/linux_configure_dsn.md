# Linux - Configuring a DSN

## Prerequisites
In order to use the Timestream ODBC Driver, [unixODBC] (http://www.unixodbc.org/) must be installed.

### Installing on Ubuntu

```
sudo apt update
sudo apt install unixodbc
```

## Adding a Driver Entry

### Manually editing odbcinst.ini ###
Use a text editor from Terminal to edit the odbcinst.ini file such as vi.

**To create a System Driver Entry run:**
`sudo vi /etc/odbcinst.ini`

**To add the driver entries:**
1. Add `Amazon Timestream ODBC Driver` in the `[ODBC Drivers]` section.
2. Add the `[Amazon Timestream ODBC Driver]` section.

#### Sample odbcinst.ini
```
[ODBC Drivers]
Amazon Timestream ODBC Driver  = Installed

[Amazon Timestream ODBC Driver]
Driver = /usr/bin/libtimestreamsqlodbc.so
Setup  = /usr/bin/libtimestreamsqlodbc.so
```

### Manually editing odbc.ini ###
Use a text editor from Terminal to edit the odbc.ini file such as vi. See [Configuration Options](./configuration_options.md) for more details on the individual entries.

**To create a System DSN Entry run:**

`sudo vi /etc/odbc.ini`

**To create a User DSN Entry run:**

`vi ~/.odbc.ini`

#### <a name="odbc_data_source"></a>Sample odbc.ini
```
[ODBC Data Sources]
timestream-iam-profile  = Amazon Timestream ODBC Driver
timestream-aad-profile  = Amazon Timestream ODBC Driver
timestream-okta-profile = Amazon Timestream ODBC Driver
timestream-aws-profile  = Amazon Timestream ODBC Driver

[timestream-aws-profile]
Driver    = Amazon Timestream ODBC Driver
Region    = us-east-2
Auth      = AWS_PROFILE

[timestream-aad-profile]
Driver           = Amazon Timestream ODBC Driver
Region           = us-east-1
Auth             = AAD
IdpName          = AzureAD
AADApplicationID = 
AADClientSecret  = 
RoleARN          = 
AADTenant        = 
IdpARN           = 
IdpUserName      = 
IdpPassword      = 

[timestream-okta-profile]
Driver            = Amazon Timestream ODBC Driver
Region            = us-east-1
Auth              = OKTA
IdpName           = Okta
IdpHost           = 
OktaApplicationID = 
RoleARN           = 
IdpARN            = 
IdpUserName       = 
IdpPassword       = 

[timestream-iam-profile]
Driver         = Amazon Timestream ODBC Driver
Region         = us-east-1
LogLevel       = 0
Auth           = IAM
UID            = 
PWD            = 
SessionToken   = 
```

## Testing Connections

Use [iSQL](https://www.systutorials.com/docs/linux/man/1-isql/#:~:text=isql%20is%20a%20command%20line,with%20built%2Din%20Unicode%20support) to test the connections and run a sample query. For example, to connect to the timestream-aws-profile DSN, 
* Type `iSQL timestream-aws-profile` to connect to the DSN.
* Once the SQL prompt appears type a sample query such as `SELECT * FROM sampleDB.IoT`. 
* Type `quit` to return to the command prompt.
