# Configuration Options

#### Driver Specific Options

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `Driver` | Driver name.| string | timestreamodbc |
| `DSN` | **D**ata **S**ource **N**ame used for configuring the connection. | string | `<NONE>` |
| `Auth` | Authentication mode. | one of `AWS_PROFILE`, `IAM`, `AAD`, `OKTA` | `AWS_PROFILE`
| `LogLevel` | Severity level for driver logs. | one of `OFF`, `FATAL`, `ERROR`, `INFO`, `DEBUG`, `TRACE`, `ALL` | `OFF` |
| `LogOutput` | Location for storing driver logs. | string | WIN: `<NONE>`, MAC/Linux: `/tmp` |

#### AWS_PROFILE Options

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `ProfileName` | Profile name for the AWS credentials. <NONE> means loading default credential chain. | string | `<NONE>` |
| `Region` | The database's region. | string |`us-east-1`|
#### AWS IAM Authentication Options

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `UID` / `AccessKeyId` | The AWS user access key id.| string | `<NONE>` |
| `PWD` / `SecretAccessKey` | The AWS user secret access key. | string | `<NONE>` |
| `SessionToken` | The temporary session token required to access a database with multi-factor authentication (MFA) enabled. | string | `<NONE>` |
| `Region` | The database's region. | string |`us-east-1`|

#### AWS SDK Options

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `RequestTimeout` | The time in milliseconds the AWS SDK will wait for a query request before timing out. Non-positive value disables request timeout. | int | `3000` |
| `ConnectionTimeout` | Socket connect timeout. Value must be non-negative. A value of 0 disables socket timeout. | int | `1000` |
| `MaxRetryCountClient` | The maximum number of retry attempts for retryable errors with 5XX error codes in the SDK. The value must be non-negative, 0 means no retry. | int | `<NONE>` |
| `MaxConnections` | The maximum number of allowed concurrently opened HTTP connections to the Timestream service. The value must be positive. | int | `25` |

#### Endpoint Configuration Options

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `EndpointOverride` | The endpoint override for the Timestream service. It overrides the region. It is an advanced option. | string | `<NONE>` |

#### SAML-based authenication options for Okta

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `IdpName` | The Identity Provider (Idp) name to use for SAML-based authentication. One of Okta or AzureAD. | string | `<NONE>` |
| `IdpHost` | The hostname of the specified Idp. | string | `<NONE>` |
| `UID` / `IdpUserName` | The username for the specified Idp account. | string | `<NONE>` |
| `PWD` / `IdpPassword` | The password for the specified Idp account. | string | `<NONE>` |
| `OktaApplicationID` | The unique Okta-provided ID associated with the Timestream application. A place to find the AppId is in the entityID field provided in the application metadata. | string | `<NONE>` |
| `RoleARN` | The Amazon Resource Name (ARN) of the role that the caller is assuming. | string | `<NONE>` |
| `IdpARN` | The Amazon Resource Name (ARN) of the SAML provider in IAM that describes the Idp. | string | `<NONE>` |

#### SAML-based authenication options for Azure AD

| Option | Description | Type | Default |
|--------|-------------|------|---------------|
| `IdpName` | The Identity Provider (Idp) name to use for SAML-based authentication. One of Okta or AzureAD. | string | `<NONE>` |
| `IdpHost` | The hostname of the specified Idp. | string | `<NONE>` |
| `UID` / `IdpUserName` | The username for the specified Idp account. | string | `<NONE>` |
| `PWD` / `IdpPassword` | The password for the specified Idp account. | string | `<NONE>` |
| `AADApplicationID` | The unique id of the registered application on Azure AD. | string | `<NONE>` |
| `AADClientSecret` | The client secret associated with the registered application on Azure AD used to authorize fetching tokens. | string | `<NONE>` |
| `AADTenant` | The Azure AD Tenant ID. | string | `<NONE>` |
| `IdpARN` | The Amazon Resource Name (ARN) of the SAML provider in IAM that describes the Idp. | string | `<NONE>` |

**NOTE:** Administrative privileges are required to change the value of logging options on Windows / macOS.