# Amazon DocumentDB ODBC Driver

## Overview

The ODBC driver for the Amazon DocumentDB managed document database provides an 
SQL-relational interface for developers and BI tool users.

## Security

See [CONTRIBUTING](CONTRIBUTING.md#security-issue-notifications) for more information.

## License

This project is licensed under the Apache-2.0 License.

## Documentation

See the [product documentation](src/markdown/index.md) for more detailed information about this driver, such as setup and configuration.

## Setup and Usage

To setup and use the DocumentDB ODBC driver, follow [these directions](src/markdown/setup/setup.md).

## Connection String Syntax

```
DRIVER={Amazon DocumentDB};HOSTNAME=<host>:<port>;DATABASE=<database>;USER=<user>;PASSWORD=<password>;<option>=<value>;
```

For more information about connecting to an Amazon DocumentDB database using this ODBC driver, see
the [connection string documentation](src/markdown/setup/connection-string.md) for more details.
