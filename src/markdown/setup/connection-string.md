# Connection String Syntax and Options
`DRIVER={Amazon DocumentDB};HOSTNAME=<host>:<port>;DATABASE=<database>;USER=<user>;PASSWORD=<password>;<option>=<value>;`

### Driver
`Driver:` Required: the driver for this ODBC driver.

### Parameters
| Property | Description | Default |
|--------|-------------|---------------|
| `DATABASE` (required) | The name of the database the ODBC driver will connect to. | `NONE`
| `HOSTNAME` (required) | The hostname or IP address of the DocumentDB server or cluster. | `NONE`
| `PORT` (optional) | The port number the DocumentDB server or cluster is listening on. | `27017`
| `USER` (optional) | The username of the authorized user. While the username is optional on the connection string, it is still required either via the connection string, or the properties. _Note: the username must be properly (%) encoded to avoid any confusion with URI special characters._ | `NONE`
| `PASSWORD` (optional) | The password of the authorized user. While the password is optional on the connection string, it is still required either via the connection string, or the properties. _Note: the password must be properly (%) encoded to avoid any confusion with URI special characters._ | `NONE`
| `OPTION` (optional) | One of the connection string options listed below. | `NONE`
| `VALUE` (optional) | The associated value for the option. | `NONE`

### Options
| Option | Description | Default |
|--------|-------------|---------------|
| `APP_NAME` | (string) Sets the logical name of the application. | `Amazon DocumentDB ODBC Driver {version}`
| `LOGIN_TIMEOUT_SEC` | (int) How long a connection can take to be opened before timing out (in seconds). Alias for connectTimeoutMS but using seconds. | `NONE`
| `READ_PREFERENCE` | (enum/string) The read preference for this connection. Allowed values: `primary`, `primaryPreferred`, `secondary`, `secondaryPreferred` or `nearest`. | `primary`
| `REPLICA_SET` | (string) Name of replica set to connect to. For now, passing a name other than `rs0` will log a warning. | `NONE`
| `RETRY_READS` | (true/false) If true, the driver will retry supported read operations if they fail due to a network error. | `true`
| `TLS` | (true/false) If true, use TLS encryption when communicating with the DocumentDB server. | `true`
| `TLS_ALLOW_INVALID_HOSTNAMES` | (true/false) If true, invalid host names for the TLS certificate are allowed. This is useful when using an internal SSH tunnel to a DocumentDB server. | `false`
| `TLS_CA_FILE` | (string) The path to the trusted Certificate Authority (CA) `.pem` file. If the path starts with the tilde character (`~`), it will be replaced with the user's home directory. Ensure to use only forward slash characters (`/`) in the path or URL encode the path. Providing the trusted Certificate Authority (CA) `.pem` file is optional as the current Amazon RDS root CA is used by default when the `tls` option is set to `true`. This embedded certificate is set to expire on 2024-08-22. For example, to provide a new trusted Certificate Authority (CA) `.pem` file that is located in the current user's `Downloads` subdirectory of their home directory, use the following: `TLS_CA_FILE=~/Downloads/rds-ca-2019-root.pem`. | `NONE`
| `SSH_USER` | (string) The username for the internal SSH tunnel. If provided, options `sshHost` and `sshPrivateKeyFile` must also be provided, otherwise this option is ignored. | `NONE`
| `SSH_HOST` | (string) The host name for the internal SSH tunnel. Optionally the SSH tunnel port number can be provided using the syntax `<ssh-host>:<port>`. The default port is `22`. If provided, options `SSH_USER` and `sshPrivateKeyFile` must also be provided, otherwise this option is ignored. | `NONE`
| `SSH_PRIVATE_KEY_FILE` | (string) The path to the private key file for the internal SSH tunnel. If the path starts with the tilde character (`~`), it will be replaced with the user's home directory. If the path is relative, the absolute path will try to be resolved by searching in the user's home directory (`~`), the `.documentdb` folder under the user's home directory or in the same directory as the driver JAR file. If the file cannot be found, a connection error will occur. If provided, options `SSH_USER` and `SSH_HOST` must also be provided, otherwise this option is ignored. | `NONE`
| `SSH_PRIVATE_KEY_PASSPHRASE` | (string) If the SSH tunnel private key file, `SSH_PRIVATE_KEY_FILE`, is passphrase protected, provide the passphrase using this option. If provided, options `SSH_USER`, `SSH_HOST` and `SSH_PRIVATE_KEY_FILE` must also be provided, otherwise this option is ignored. | `NONE`
| `SSH_STRICT_HOST_KEY_CHECKING` | (true/false) If true, the 'known_hosts' file is checked to ensure the target host is trusted when creating the internal SSH tunnel. If false, the target host is not checked. Disabling this option is less secure as it can lead to a ["man-in-the-middle" attack](https://en.wikipedia.org/wiki/Man-in-the-middle_attack). If provided, options `sshUser`, `sshHost` and `SSH_PRIVATE_KEYFILE` must also be provided, otherwise this option is ignored. | `true`
| `SSH_KNOWN_HOSTS_FILE` | (string) The path to the 'known_hosts' file used for checking the target host for the SSH tunnel when option `SSH_STRICT_HOST_KEY_CHECKING` is `true`. The `known_hosts` file can be populated using the `ssh-keyscan` [tool](https://github.com/aws/amazon-documentdb-jdbc-driver/blob/develop/src/markdown/setup/maintain_known_hosts.md). If provided, options `SSH_USER`, `SSH_HOST` and `SSH_PRIVATE_KEY_FILE` must also be provided, otherwise this option is ignored. | `~/.ssh/known_hosts`
| `SCAN_METHOD` | (enum/string) The scanning (sampling) method to use when discovering collection metadata for determining table schema. Possible values include the following: 1) `RANDOM` - the sample documents are returned in _random_ order, 2) `ID_FORWARD` - the sample documents are returned in order of id, 3) `ID_REVERSE` - the sample documents are returned in reverse order of id or 4) `ALL` - sample all the documents in the collection. | `RANDOM`
| `SCAN_LIMIT` | (int) The number of documents to sample. The value must be a positive integer. If `SCAN_METHOD` is set to `all`, this option is ignored. | `1000`
| `SCHEMA_NAME` | (string) The name of the SQL mapping schema for the database. | `_default`.  
| `DEFAULT_FETCH_SIZE` | (int) The default fetch size (in records) when retrieving results from Amazon DocumentDB. It is the number of records to retrieve in a single batch. The maximum number of records retrieved in a single batch may also be limited by the overall memory size of the result. | `2000`
| `REFERESH_SCHEMA` | (true/false) If true, generates (refreshes) the SQL schema with each connection. It creates a new version, leaving any existing versions in place. _Caution: use only when necessary to update schema as it can adversely affect performance._  | `false`

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
DRIVER={Amazon DocumentDB};HOSTNAME=localhost:27017;DATABASE=customer;TLS_ALLOW_INVALID_HOSTNAMES=true;
```

#### Notes:

1. An external [SSH tunnel](setup.md#using-an-ssh-tunnel-to-connect-to-amazon-documentdb) is being used where the local 
port is `27117`.
1. The Amazon DocumentDB database name is `customer`.
1. The Amazon DocumentDB is TLS-enabled (`tls=true` is default).
1. User and password values are passed to the ODBC driver using **Properties**.

### Connecting to an Amazon DocumentDB Cluster using an Internal SSH tunnel

```
DRIVER={Amazon DocumentDB};HOSTNAME=localhost:27017;DATABASE=customer;TLS_ALLOW_INVALID_HOSTNAMES=true;SSH_USER=ec2-user;SSH_HOST=ec2-254-254-254-254.compute.amazonaws.com;SSH_PRIVATE_KEY_FILE=~/.ssh/ec2-privkey.pem
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
DRIVER={Amazon DocumentDB};HOSTNAME=localhost:27017;DATABASE=customer;TLS_ALLOW_INVALID_HOSTNAMES=true;SCAN_METHOD=ID_FORWARD;SCAN_LIMIT=5000
```

#### Notes:

1. An external [SSH tunnel](setup.md#using-an-ssh-tunnel-to-connect-to-amazon-documentdb) is being used where the 
local port is `27017` (`27017` is default).
2. The Amazon DocumentDB database name is `customer`.
3. The Amazon DocumentDB is TLS-enabled (`tls=true` is default).
4. User and password values are passed to the ODBC driver using **Properties**.
5. The scan method `ID_FORWARD` will order the result using the `_id` column in the collection.
6. The scan limit `5000` will limit the number of scanned documents to 5000.
