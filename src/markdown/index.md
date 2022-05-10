# Amazon DocumentDB ODBC Driver Documentation

## Overview

The ODBC driver for the Amazon DocumentDB managed document database provides an
SQL-relational interface for developers and BI tool users.

## License

This project is licensed under the Apache-2.0 License.

## Documentation

- Setup
    - [Amazon DocumentDB ODBC Driver Setup](setup/setup.md)
    - [DSN](setup/dsn-configuration.md)
- Managing Schema
    - [Schema Discovery and Generation](https://github.com/aws/amazon-documentdb-jdbc-driver/blob/develop/src/markdown/schema/schema-discovery.md)
    - [Managing Schema Using the Command Line Interface](https://github.com/aws/amazon-documentdb-jdbc-driver/blob/develop/src/markdown/schema/manage-schema-cli.md)
    - [Table Schemas JSON Format](https://github.com/aws/amazon-documentdb-jdbc-driver/blob/develop/src/markdown/schema/table-schemas-json-format.md)
- SQL Compatibility
    - [SQL Support and Limitations](https://github.com/aws/amazon-documentdb-jdbc-driver/blob/develop/src/markdown/sql/sql-limitations.md)
- Support
    - [Troubleshooting Guide](support/troubleshooting-guide.md)
  
## Getting Started

Follow the [requirements and setup directions](setup/setup.md) to get you environment ready to use the
Amazon DocumentDB ODBC driver. Assuming your Amazon DocumentDB cluster is hosted in a private VPC, 
you'll want to [create an SSH tunnel](setup/setup.md#using-an-ssh-tunnel-to-connect-to-amazon-documentdb) to bridge to 
your cluster in the VPC. If you're a Tableau or other BI user, follow the directions on how to 
[setup and use BI tools](setup/setup.md#driver-setup-in-bi-applications) with the driver.

## Setup and Usage

To set up and use the DocumentDB JDBC driver, see [Amazon DocumentDB ODBC Driver Setup](setup/setup.md).

## Connection String Syntax

```
DRIVER={Amazon DocumentDB};[HOSTNAME=<host>:<port>];[DATABASE=<database>];[USER=<user>];[PASSWORD=<password>][;<option>=<value>[;<option>=<value>[...]]];
```

For more information about connecting to an Amazon DocumentDB database using this ODBC driver, see
the [dsn configuration](setup/dsn-configuration.md) for more details.
## Schema Discovery

The Amazon DocumentDB ODBC driver can perform automatic schema discovery and generate an SQL to
DocumentDB schema mapping. See the [schema discovery documentation](https://github.com/aws/amazon-documentdb-jdbc-driver/blob/develop/src/markdown/schema/schema-discovery.md)
for more details of this process.

## Schema Management

The SQL to DocumentDB schema mapping can be managed using JDBC command line in the following ways:

- generated
- removed
- listed
- exported
- imported

See the [schema management documentation](https://github.com/aws/amazon-documentdb-jdbc-driver/blob/develop/src/markdown/schema/manage-schema-cli.md) and
[table schemas JSON format](https://github.com/aws/amazon-documentdb-jdbc-driver/blob/develop/src/markdown/schema/table-schemas-json-format.md) for further
information.

**Note**: A common schema management task is to regenerate or clear the existing schema that has
become out of date when your database has changed, for example, when there are new collections or new
fields in an existing collection. To regenerate or clear the existing schema, please refer to the
[Schema Out of Date](#schema-out-of-date) topic in the troubleshooting guide.

## SQL and ODBC Limitations

The Amazon DocumentDB ODBC driver has a number of important limitations. See the
[SQL limitations documentation](https://github.com/aws/amazon-documentdb-jdbc-driver/blob/develop/src/markdown/sql/sql-limitations.md).

## Troubleshooting Guide

If you're having an issue using the Amazon DocumentDB ODBC driver, consult the
[Troubleshooting Guide](support/troubleshooting-guide.md) to see if it has a solution for
your issue.
