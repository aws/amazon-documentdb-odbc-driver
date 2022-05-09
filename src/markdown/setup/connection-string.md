# Connection String Syntax and Options
`DRIVER={Amazon DocumentDB};HOSTNAME=<host>:<port>;DATABASE=<database>;USER=<user>;PASSWORD=<password>;<option>=<value>;`

### Driver
`Driver:` Required: the driver for this ODBC driver.

### Parameters
| Property | Description | Default |
|--------|-------------|---------------|
| `HOSTNAME` (required) | The hostname or IP address of the DocumentDB server or cluster. | `NONE`
| `PORT` (optional) | The port number the DocumentDB server or cluster is listening on. | `27017`
| `DATABASE` (required) | The name of the database the JDBC driver will connect to. | `NONE`
| `USER` (optional) | The username of the authorized user. While the username is optional on the connection string, it is still required either via the connection string, or the properties. _Note: the username must be properly (%) encoded to avoid any confusion with URI special characters._ | `NONE`
| `PASSWORD` (optional) | The password of the authorized user. While the password is optional on the connection string, it is still required either via the connection string, or the properties. _Note: the password must be properly (%) encoded to avoid any confusion with URI special characters._ | `NONE`
| `OPTION` (optional) | One of the connection string options listed below. | `NONE`
| `VALUE` (optional) | The associated value for the option. | `NONE`

### Options
| Option | Description | Default |
|--------|-------------|---------------|
| `APPNAME` | (string) Sets the logical name of the application. | `Amazon DocumentDB JDBC Driver {version}`
| `LOGINTIMEOUTSEC` | (int) How long a connection can take to be opened before timing out (in seconds). Alias for connectTimeoutMS but using seconds. | `NONE`
| `READPREFERENCE` | (enum/string) The read preference for this connection. Allowed values: `primary`, `primaryPreferred`, `secondary`, `secondaryPreferred` or `nearest`. | `primary`
| `REPLICASET` | (string) Name of replica set to connect to. For now, passing a name other than `rs0` will log a warning. | `NONE`
| `RETRYREADS` | (true/false) If true, the driver will retry supported read operations if they fail due to a network error. | `true`
| `TLS` | (true/false) If true, use TLS encryption when communicating with the DocumentDB server. | `true`
| `TLSALLOWINVALIDHOSTNAMES` | (true/false) If true, invalid host names for the TLS certificate are allowed. This is useful when using an internal SSH tunnel to a DocumentDB server. | `false`
| `TLSCAFILE` | (string) The path to the trusted Certificate Authority (CA) `.pem` file. If the path starts with the tilde character (`~`), it will be replaced with the user's home directory. Ensure to use only forward slash characters (`/`) in the path or URL encode the path. Providing the trusted Certificate Authority (CA) `.pem` file is optional as the current Amazon RDS root CA is used by default when the `tls` option is set to `true`. This embedded certificate is set to expire on 2024-08-22. For example, to provide a new trusted Certificate Authority (CA) `.pem` file that is located in the current user's `Downloads` subdirectory of their home directory, use the following: `tlsCAFile=~/Downloads/rds-ca-2019-root.pem`. | `NONE`
| `SSHUSER` | (string) The username for the internal SSH tunnel. If provided, options `sshHost` and `sshPrivateKeyFile` must also be provided, otherwise this option is ignored. | `NONE`
| `SSHHOST` | (string) The host name for the internal SSH tunnel. Optionally the SSH tunnel port number can be provided using the syntax `<ssh-host>:<port>`. The default port is `22`. If provided, options `sshUser` and `sshPrivateKeyFile` must also be provided, otherwise this option is ignored. | `NONE`
| `SSHPRIVATEKEYFILE` | (string) The path to the private key file for the internal SSH tunnel. If the path starts with the tilde character (`~`), it will be replaced with the user's home directory. If the path is relative, the absolute path will try to be resolved by searching in the user's home directory (`~`), the `.documentdb` folder under the user's home directory or in the same directory as the driver JAR file. If the file cannot be found, a connection error will occur. If provided, options `sshUser` and `sshHost` must also be provided, otherwise this option is ignored. | `NONE`
| `SSHPRIVATEKEYPASSPHRASE` | (string) If the SSH tunnel private key file, `sshPrivateKeyFile`, is passphrase protected, provide the passphrase using this option. If provided, options `sshUser`, `sshHost` and `sshPrivateKeyFile` must also be provided, otherwise this option is ignored. | `NONE`
| `sshStrictHostKeyChecking` | (true/false) If true, the 'known_hosts' file is checked to ensure the target host is trusted when creating the internal SSH tunnel. If false, the target host is not checked. Disabling this option is less secure as it can lead to a ["man-in-the-middle" attack](https://en.wikipedia.org/wiki/Man-in-the-middle_attack). If provided, options `sshUser`, `sshHost` and `sshPrivateKeyFile` must also be provided, otherwise this option is ignored. | `true`
| `SSHKNOWNHOSTFILE` | (string) The path to the 'known_hosts' file used for checking the target host for the SSH tunnel when option `sshStrictHostKeyChecking` is `true`. The `known_hosts` file can be populated using the `ssh-keyscan` [tool](maintain_known_hosts.md). If provided, options `sshUser`, `sshHost` and `sshPrivateKeyFile` must also be provided, otherwise this option is ignored. | `~/.ssh/known_hosts`
| `SCANMETHOD` | (enum/string) The scanning (sampling) method to use when discovering collection metadata for determining table schema. Possible values include the following: 1) `random` - the sample documents are returned in _random_ order, 2) `idForward` - the sample documents are returned in order of id, 3) `idReverse` - the sample documents are returned in reverse order of id or 4) `all` - sample all the documents in the collection. | `random`
| `scanLimit` | (int) The number of documents to sample. The value must be a positive integer. If `scanMethod` is set to `all`, this option is ignored. | `1000`
| `SCHEMANAME` | (string) The name of the SQL mapping schema for the database. | `_default`.  
| `DEFAULTFETCHSIZE` | (int) The default fetch size (in records) when retrieving results from Amazon DocumentDB. It is the number of records to retrieve in a single batch. The maximum number of records retrieved in a single batch may also be limited by the overall memory size of the result. The value can be changed by calling the `Statement.setFetchSize` JDBC method. | `2000`
| `REFERESHSCHEMA` | (true/false) If true, generates (refreshes) the SQL schema with each connection. It creates a new version, leaving any existing versions in place. _Caution: use only when necessary to update schema as it can adversely affect performance._  | `false`
| `DEAFAULTAUTHDB` | (string) The name of the authentication database to use when authenticating with the passed `user` and `password`. This is where the authorized user is stored and can be different from what databases the user may have access to. On Amazon DocumentDB, all users are attributed to the `admin` database. | `admin`

## Examples

### Connecting to an Amazon DocumentDB Cluster

```
DRIVER={Amazon DocumentDB};HOSTNAME=localhost;DATABASE=customer;TLSALLOWINVALIDHOSTNAMES=true;
```

#### Notes:

1. An external [SSH tunnel](setup.md#using-an-ssh-tunnel-to-connect-to-amazon-documentdb) is being used where the local 
port is `27017` (`27017` is default).
2. The Amazon DocumentDB database name is `customer`.
3. The Amazon DocumentDB is TLS-enabled (`tls=true` is default)
4. User and password values are passed to the ODBC driver using **Properties**.

### Connecting to an Amazon DocumentDB Cluster on Non-Default Port

```
DRIVER={Amazon DocumentDB};HOSTNAME=localhost:27017;DATABASE=customer;TLSALLOWINVALIDHOSTNAMES=true;
```

#### Notes:

1. An external [SSH tunnel](setup.md#using-an-ssh-tunnel-to-connect-to-amazon-documentdb) is being used where the local 
port is `27117`.
1. The Amazon DocumentDB database name is `customer`.
1. The Amazon DocumentDB is TLS-enabled (`tls=true` is default).
1. User and password values are passed to the ODBC driver using **Properties**.

### Connecting to an Amazon DocumentDB Cluster using an Internal SSH tunnel

```
DRIVER={Amazon DocumentDB};HOSTNAME=localhost:27017;DATABASE=customer;TLSALLOWINVALIDHOSTNAMES=true;SSHUSER=ec2-user;SSHHOST=ec2-254-254-254-254.compute.amazonaws.com;SSHPRIVATEKEYFILE=~/.ssh/ec2-privkey.pem
```

#### Notes:

1. DocumentDB cluster host is `docdb-production.docdb.amazonaws.com` (using default port `27017`).
2. The Amazon DocumentDB database name is `customer`.
3. The Amazon DocumentDB is TLS-enabled (`tls=true` is default).
4. An internal SSH tunnel will be created using the user `ec2-user`,
   host `ec2-254-254-254-254.compute.amazonaws.com`, and private key file `~/.ssh/ec2-privkey.pem`.
6. User and password values are passed to the ODBC driver using **Properties**.

### Change the Scanning Method when Connecting to an Amazon DocumentDB Cluster

```
DRIVER={Amazon DocumentDB};HOSTNAME=localhost:27017;DATABASE=customer;TLSALLOWINVALIDHOSTNAMES=true;SCANMETHOD=idForward;SCANLIMIT=5000
```

#### Notes:

1. An external [SSH tunnel](setup.md#using-an-ssh-tunnel-to-connect-to-amazon-documentdb) is being used where the 
local port is `27017` (`27017` is default).
2. The Amazon DocumentDB database name is `customer`.
3. The Amazon DocumentDB is TLS-enabled (`tls=true` is default).
4. User and password values are passed to the ODBC driver using **Properties**.
5. The scan method `idForward` will order the result using the `_id` column in the collection.
6. The scan limit `5000` will limit the number of scanned documents to 5000.
