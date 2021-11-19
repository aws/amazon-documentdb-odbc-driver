# Linux - Configuring a DSN

## Prerequisites
In order to use the Database ODBC Driver, [unixODBC](http://www.unixodbc.org/) must be installed.

### Installing on Ubuntu 64 bit

```
sudo apt update
sudo apt install unixodbc
```

### Installing on Ubuntu 32 bit

```
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install unixodbc:i386
```

### Installing on Amazon Linux 2 64 bit

```
sudo yum update
sudo yum install unixODBC
```

## Adding a Driver Entry

### Manually editing the odbcinst.ini file ###
Use a text editor from the Unix shell to edit the odbcinst.ini file such as vi.

**To create a System Driver Entry run:**
`sudo vi /etc/odbcinst.ini`

**To add the driver entries:**
1. Add `Database ODBC Driver` in the `[ODBC Drivers]` section.
2. Add the `[Database ODBC Driver]` section.

#### Sample odbcinst.ini file for 64-bit Linux
```
[ODBC Drivers]
Database ODBC Driver  = Installed

[Database ODBC Driver]
Driver = /usr/bin/database-odbc64/libodbcdriver.so
Setup  = /usr/bin/database-odbc64/libodbcdriver.so
```

#### Sample odbcinst.ini file for 32-bit Linux
```
[ODBC Drivers]
Database ODBC Driver 32 = Installed

[Database ODBC Driver 32]
Driver = /usr/bin/database-odbc32/libodbcdriver.so
Setup  = /usr/bin/database-odbc32/libodbcdriver.so
```

### Manually editing the odbc.ini file ###
Use a text editor from the Unix shell to edit the odbc.ini file such as vi. See [Configuration Options](./configuration_options.md) for more details on the individual entries.

**To create a System DSN Entry run:**

`sudo vi /etc/odbc.ini`

**To create a User DSN Entry run:**

`vi ~/.odbc.ini`

#### <a name="odbc_data_source"></a>Sample odbc.ini file
```
[ODBC Data Sources]
database-default     = Database ODBC Driver
database-default-32  = Database ODBC Driver 32

[database-default-32]
Driver    = Database ODBC Driver 32
Auth      = DEFAULT
UID       = 
PWD       = 

[database-default]
Driver    = Database ODBC Driver
Auth      = DEFAULT
UID       = 
PWD       = 
```

## Testing Connections

### Testing the 64-bit Database ODBC Driver for Linux
Use [isql](https://www.systutorials.com/docs/linux/man/1-isql/#:~:text=isql%20is%20a%20command%20line,with%20built%2Din%20Unicode%20support) to test the connections and run a sample query. For example, to connect to the database-default DSN, 
* Type `isql database-default` to make a connection and enter interactive mode.

* Once the SQL prompt appears type a sample query such as `SELECT * FROM sampleDB.IoT`. 
* Type `quit` to exit interactive mode.

### Testing the 32-bit Database ODBC Driver for Linux
You can use [isql32](../../tools/isql32) for testing. In the directory where it is located:
* Type `chmod +x ./isql32` to make it executable.
* Type `./isql32 [DSN]` to make a connection and enter interactive mode.
* Once the SQL prompt appears type a sample query such as `SELECT * FROM sampleDB.IoT`.
* Type `quit` to exit interactive mode.
