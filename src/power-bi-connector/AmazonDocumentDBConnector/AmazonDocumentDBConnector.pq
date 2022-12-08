section AmazonDocumentDBConnector;

// When set to true, additional trace information will be written out to the User log. 
// This should be set to false before release. Tracing is done through a call to 
// Diagnostics.LogValue(). When EnableTraceOutput is set to false, the call becomes a 
// no-op and simply returns the original value.
EnableTraceOutput = false;

[DataSource.Kind="AmazonDocumentDBConnector", Publish="AmazonDocumentDBConnector.Publish"]
shared AmazonDocumentDBConnector.Contents = Value.ReplaceType(AmazonDocumentDBConnectorImpl, AmazonDocumentDBConnectorType);

// Wrapper function to provide additional UI customization.
AmazonDocumentDBConnectorType = type function (
        hostname as (type text meta [
            Documentation.FieldCaption = "HostName",
            Documentation.FieldDescription = "DocumentDB host name. Default: None",
            Documentation.SampleValues = { "localhost" }
        ]),
        port as (type number meta [
            Documentation.FieldCaption = "Port",
            Documentation.FieldDescription = "DocumentDB port number. Default: None",
            Documentation.SampleValues = { "27017" }
        ]),
        database as (type text meta [
            Documentation.FieldCaption = "Database",
            Documentation.FieldDescription = "DocumentDB database name.  Default: None",
            Documentation.SampleValues = { "database" }
        ]),
        optional tls as (type logical meta [
            Documentation.FieldCaption = "TLS",
            Documentation.FieldDescription = "TLS.  Default: true",
            Documentation.AllowedValues = {true, false},
            Documentation.SampleValues = { "true" },
            DataSource.Path = true
            
        ]),
        optional tls_allow_invalid_hostnames as (type logical meta [
            Documentation.FieldCaption = "Allow Invalid HostNames",
            Documentation.FieldDescription = "Allow Invalid HostNames.  Default: false",
            Documentation.AllowedValues = {true, false},
            Documentation.DefaultValue = {false},
            Documentation.SampleValues = { "true" },
            DataSource.Path = true
        ]),
        optional tls_ca_file as (type text meta [
            Documentation.FieldCaption = "TLS CA File Path",
            Documentation.FieldDescription = "TLS CA file path.  Default: None",
            Documentation.SampleValues = { "c:\Users\example\.ssh\CA.pem" },
            DataSource.Path = true
        ]),
        optional enable_ssh_tunnel as (type logical meta [
            Documentation.FieldCaption = "Enable SSH tunnel",
            Documentation.FieldDescription = "Enable SSH tunnel.  Default: false",
            Documentation.AllowedValues = {true, false},
            Documentation.DefaultValue = {false},
            Documentation.SampleValues = { "true" },
            DataSource.Path = true
        ]),
        optional ssh_tunnel_user as (type text meta [
            Documentation.FieldCaption = "SSH tunnel user",
            Documentation.FieldDescription = "SSH tunnel user.  Default: None",
            Documentation.SampleValues = { "ec2_username" },
            DataSource.Path = true
        ]),
        optional ssh_tunnel_hostname as (type text meta [
            Documentation.FieldCaption = "SSH tunnel hostname",
            Documentation.FieldDescription = "SSH tunnel hostname.  Default: None",
            Documentation.SampleValues = { "ec2_hostname" },
            DataSource.Path = true
        ]),
        optional ssh_tunnel_private_cert as (type text meta [
            Documentation.FieldCaption = "SSH tunnel private certificate path",
            Documentation.FieldDescription = "SSH tunnel private certificate path.  Default: None",
            Documentation.SampleValues = { "c:\Users\User\.ssh\ec2_cert.pem" },
            DataSource.Path = true
        ]),
        optional ssh_tunnel_restrict_host_key_check as (type logical meta [
            Documentation.FieldCaption = "SSH Strict Host Key Check",
            Documentation.FieldDescription = "Disabling this option is less secure.  Default: true",
            Documentation.AllowedValues = {true, false},
            Documentation.DefaultValue = {false},
            Documentation.SampleValues = { "true" },
            DataSource.Path = true
        ]),
        optional ssh_tunnel_known_hosts_file as (type text meta [
            Documentation.FieldCaption = "SSH Known Hosts file",
            Documentation.FieldDescription = "Path for the ts SSH known host file.  Default: %USERPROFILE%/.ssh/known_hosts",
            Documentation.SampleValues = { "c:\Users\example\.ssh\known_hosts" },
            DataSource.Path = true
        ]),
        optional schema_scan_method as (type text meta [
            Documentation.FieldCaption = "Scan Method",
            Documentation.FieldDescription = "Scan Method.  Default: Random",
            Documentation.AllowedValues = {"Random", "ID Forward", "ID Reverse", "All"},
            Documentation.DefaultValue = {"Random"},
            Documentation.SampleValues = { "Random" },
            DataSource.Path = true
        ]),
        optional schema_scan_limit as (type number meta [
            Documentation.FieldCaption = "Scan Limit",
            Documentation.FieldDescription = "Scan Limit.  Default: 1000",
            Documentation.SampleValues = { "1000" },
            DataSource.Path = true
        ]),
        optional schema_name as (type text meta [
            Documentation.FieldCaption = "Schema Name",
            Documentation.FieldDescription = "Schema Name.  Default: _default",
            Documentation.SampleValues = { "_default" },
            DataSource.Path = true
        ]),
        optional refresh_schema as (type logical meta [
            Documentation.FieldCaption = "Refresh Schema",
            Documentation.FieldDescription = "Caution use temporarly to refresh schema.  Default: false",
            Documentation.AllowedValues = {true, false},
            Documentation.DefaultValue = {false},
            Documentation.SampleValues = { "true" },
            DataSource.Path = true
        ]),
        optional log_level as (type text meta [
            Documentation.FieldCaption = "Log Level",
            Documentation.FieldDescription = "Log Level.  Default: Error",
            Documentation.AllowedValues = {"Debug", "Info", "Error", "Off"},
            Documentation.DefaultValue = {"Error"},
            Documentation.SampleValues = { "Error" },
            DataSource.Path = true
        ]),
        optional log_path as (type text meta [
            Documentation.FieldCaption = "Log Path",
            Documentation.FieldDescription = "Log Path.  Default: %USERPROFILE%",
            Documentation.SampleValues = { "c:\Users\Example\" },
            DataSource.Path = true
        ]),

        optional retry_reads as (type logical meta [
            Documentation.FieldCaption = "Retry Reads",
            Documentation.FieldDescription = "Retry Reads. Default: true",
            Documentation.AllowedValues = {true, false},
            Documentation.DefaultValue = {false},
            Documentation.SampleValues = { "true" },
            DataSource.Path = true
        ]),
        optional read_preference as (type text meta [
            Documentation.FieldCaption = "Read Preference",
            Documentation.FieldDescription = "Read Preference. Default: primary",
            Documentation.SampleValues = { "Primary" },
            DataSource.Path = true
        ]),
        optional logging_timeout as (type number meta [
            Documentation.FieldCaption = "Logging Timeout (s)",
            Documentation.FieldDescription = "Logging Timeout (s)",
            Documentation.SampleValues = { "0" },
            DataSource.Path = true
        ]),
        optional replica_set as (type text meta [
            Documentation.FieldCaption = "Replica Set",
            Documentation.FieldDescription = "Replica Set. Default: None",
            Documentation.SampleValues = { "rc:2" },
            DataSource.Path = true
        ]),
        optional fetch_size as (type number meta [
            Documentation.FieldCaption = "Fetch Size",
            Documentation.FieldDescription = "Fetch size. Default: 2000",
            Documentation.SampleValues = { "2000" },
            DataSource.Path = true
        ])
    )
    as table meta [
        Documentation.Name = "Amazon DocumentDB"
    ];

AmazonDocumentDBConnectorImpl = (
    hostname as text,
    port as number,
    database as text,
    optional tls as logical,
    optional tls_allow_invalid_hostnames as logical ,
    optional tls_ca_file as text,
    optional enable_ssh_tunnel as logical,
    optional ssh_tunnel_user as text ,
    optional ssh_tunnel_hostname as text ,
    optional ssh_tunnel_private_cert as text,
    optional ssh_tunnel_restrict_host_key_check as logical,
    optional ssh_tunnel_known_hosts_file as text,
    optional schema_scan_method as text,
    optional schema_scan_limit as number,
    optional schema_name as text ,
    optional refresh_schema as logical,
    optional log_level as text,
    optional log_path as text,
    optional retry_reads as logical,
    optional read_preference as text,
    optional logging_timeout as number,
    optional replica_set as text,
    optional fetch_size as number
    ) as table =>
    let
        Credential = Extension.CurrentCredential(),
        AuthenticationMode = Credential[AuthenticationKind],

        // Sets connection string properties for encrypted connections.
        EncryptedConnectionString =
            if Credential[EncryptConnection] = null or Credential[EncryptConnection] = true then
                [
                    SSL = 1
                ]
            else
                [
                    SSL = 0
                ],


        AuthenticationString = 
            if AuthenticationMode = "UsernamePassword" then
                [
                    USER = Credential[Username],
                    PASSWORD = Credential[Password],
                    Auth = "BASIC"
                ]
            else [ ],

        BaseConnectionString = [
            Driver = "Amazon DocumentDB",
            App_Name = "Amazon DocumentDB PowerBI Connector ODBC Driver",
            Database = database,
            Default_fetch_size = if fetch_size = null then 2000 else fetch_size,
            Hostname =  hostname,
            Log_level = if log_level = null then "Error" else log_level,
            Log_path = if log_path = null then "" else log_path,
            Login_timeout_sec = if logging_timeout = null then 0 else logging_timeout,
            Port = port,
            Read_preference = if read_preference = null then "primary" else read_preference,
            Refresh_schema = if refresh_schema = null  then "false" else "true",
            Replica_set = if replica_set = null then "" else replica_set,
            Retry_reads = if retry_reads = null or retry_reads = true then "true" else "false",
            Scan_limit = if schema_scan_limit = null then 1000 else schema_scan_limit ,
            Scan_method = if schema_scan_method = null then "random" else schema_scan_method,
            Schema_name = if schema_name = null then "_default" else schema_name,
            Ssh_enable = if enable_ssh_tunnel = null or enable_ssh_tunnel = false then "false" else "true",
            Ssh_host = if ssh_tunnel_hostname = null then "" else ssh_tunnel_hostname,
            Ssh_known_hosts_file = if ssh_tunnel_known_hosts_file = null then "" else ssh_tunnel_known_hosts_file,
            Ssh_private_key_file = if ssh_tunnel_private_cert = null then "" else ssh_tunnel_private_cert,
            Ssh_strict_host_key_checking = if ssh_tunnel_restrict_host_key_check = null or ssh_tunnel_restrict_host_key_check = true then "true" else "false",
            Ssh_user = if ssh_tunnel_user = null then "" else ssh_tunnel_user,
            Tls = if tls = null or tls = true then "true" else "false" ,
            Tls_allow_invalid_hostnames = if tls_allow_invalid_hostnames = null  then "false" else "true",
            Tls_ca_file = if tls_ca_file = null then "" else tls_ca_file
        ],

        ConnectionString = BaseConnectionString & AuthenticationString,

        SqlCapabilities = Diagnostics.LogValue("SqlCapabilities_Options", [
            SupportsTop = false,
            LimitClauseKind = LimitClauseKind.LimitOffset,
            Sql92Conformance = ODBC[SQL_SC][SQL_SC_SQL92_FULL],
            SupportsNumericLiterals = true,
            SupportsStringLiterals = true,
            SupportsOdbcDateLiterals = true,
            SupportsOdbcTimeLiterals = true,
            SupportsOdbcTimestampLiterals = true
        ]),

        // SQLColumns is a function handler that receives the results of an ODBC call to SQLColumns().
        SQLColumns = (catalogName, schemaName, tableName, columnName, source) =>
            if (EnableTraceOutput <> true) then source else
            // the if statement conditions will force the values to evaluated/written to diagnostics
            if (Diagnostics.LogValue("SQLColumns.TableName", tableName) <> "***" and Diagnostics.LogValue("SQLColumns.ColumnName", columnName) <> "***") then
                let
                    // Outputting the entire table might be too large, and result in the value being truncated.
                    // We can output a row at a time instead with Table.TransformRows()
                    rows = Table.TransformRows(source, each Diagnostics.LogValue("SQLColumns", _)),
                    toTable = Table.FromRecords(rows)
                in
                    Value.ReplaceType(toTable, Value.Type(source))
            else
                source,

        SQLGetInfo = Diagnostics.LogValue("SQLGetInfo_Options", [
            SQL_AGGREGATE_FUNCTIONS = ODBC[SQL_AF][All],
            SQL_SQL_CONFORMANCE = ODBC[SQL_SC][SQL_SC_SQL92_INTERMEDIATE]
        ]),

        SQLGetFunctions = Diagnostics.LogValue("SQLGetFunctions_Options", [
            SQL_API_SQLBINDPARAMETER = false
        ]),

        SQLGetTypeInfo = (types) => 
            if (EnableTraceOutput <> true) then types else
            let
                // Outputting the entire table might be too large, and result in the value being truncated.
                // We can output a row at a time instead with Table.TransformRows()
                rows = Table.TransformRows(types, each Diagnostics.LogValue("SQLGetTypeInfo " & _[TYPE_NAME], _)),
                toTable = Table.FromRecords(rows)
            in
                Value.ReplaceType(toTable, Value.Type(types)),

        AstVisitor = [
            Constant =
                let
                    Quote = each Text.Format("'#{0}'", { _ }),
                    Cast = (value, typeName) => [
                        Text = Text.Format("CAST(#{0} as #{1})", { value, typeName })
                    ],
                    Visitor = [
                        timestamp = each Cast(Quote(DateTime.ToText(_, "yyyy-MM-dd HH:mm:ss.fffffff")), "timestamp")
                    ]
                in
                    (typeInfo, ast) => Record.FieldOrDefault(Visitor, typeInfo[TYPE_NAME], each null)(ast[Value])
        ],
        
        ConnectionOptions = [
            // View the tables grouped by their schema names.
            HierarchicalNavigation = true,
            
            // Handlers for ODBC driver capabilities.
            SqlCapabilities = SqlCapabilities,
            SQLColumns = SQLColumns,
            SQLGetInfo = SQLGetInfo,
            SQLGetFunctions = SQLGetFunctions,
            SQLGetTypeInfo = SQLGetTypeInfo,

            // For modifying generated SQL.
            AstVisitor = AstVisitor,
            
            // Handles ODBC Errors.
            OnError = OnOdbcError,

            // Connection string properties used for encrypted connections.
            CredentialConnectionString = EncryptedConnectionString
        ],

        OdbcDatasource = Odbc.DataSource(ConnectionString, ConnectionOptions)
    in
        OdbcDatasource;

// Handles ODBC errors.
OnOdbcError = (errorRecord as record) =>
    let
        ErrorMessage = errorRecord[Message],

        IsDriverNotInstalled = Text.Contains(ErrorMessage, "doesn't correspond to an installed ODBC driver"),

        OdbcError = errorRecord[Detail][OdbcErrors]{0},
        OdbcErrorCode = OdbcError[NativeError],
        
        // Failed to connect to given host.
        IsHostUnreachable = (OdbcErrorCode = 202)
    in
        if IsDriverNotInstalled then
            error Error.Record(errorRecord[Reason], "The Amazon DocumentDB ODBC driver is not installed. Please install the driver.", errorRecord[Detail])
        else if IsHostUnreachable then
            error Error.Record(errorRecord[Reason], "Could not reach server. Please double-check the authentication.", errorRecord[Detail])
        else 
            error errorRecord;

// Data Source Kind description.
AmazonDocumentDBConnector = [
    Label = Extension.LoadString("DataSourceLabel"),

    SupportsEncryption = true,
    Authentication = [
        UsernamePassword = [
            UsernameLabel = Extension.LoadString("UsernameLabel"),
            PasswordLabel = Extension.LoadString("PasswordLabel"),
            Label = "DocumentDB Credentials"
        ]
    ],
    
    // Needed for use with Power BI Service.
    TestConnection = (dataSourcePath) => 
        let
            json = Json.Document(dataSourcePath),
            hostname = json[hostname],
            port = json[port],
            database = json[database],
            tls = json[tls],
            tls_allow_invalid_hostnames = json[tls_allow_invalid_hostnames] ,
            tls_ca_file = json[tls_ca_file],
            enable_ssh_tunnel = json[enable_ssh_tunnel],
            ssh_tunnel_user = json[ssh_tunnel_user] ,
            ssh_tunnel_hostname = json[ssh_tunnel_hostname] ,
            ssh_tunnel_private_cert = json[ssh_tunnel_private_cert],
            ssh_tunnel_restrict_host_key_check = json[ssh_tunnel_restrict_host_key_check],
            ssh_tunnel_known_hosts_file = json[ssh_tunnel_known_hosts_file],
            schema_scan_method = json[schema_scan_method],
            schema_scan_limit = json[schema_scan_limit],
            schema_name = json[schema_name] ,
            refresh_schema = json[refresh_schema],
            log_level = json[log_level],
            log_path = json[log_path],
            retry_reads = json[retry_reads],
            read_preference = json[read_preference],
            logging_timeout = json[logging_timeout],
            replica_set = json[replica_set],
            fetch_size = json[fetch_size]
        in
            { "AmazonDocumentDBConnector.Contents", 
            hostname, 
            port, 
            database,
            tls, 
            tls_allow_invalid_hostnames, 
            tls_ca_file, 
            enable_ssh_tunnel, 
            ssh_tunnel_user, 
            ssh_tunnel_hostname,
            ssh_tunnel_private_cert,
            ssh_tunnel_restrict_host_key_check,
            ssh_tunnel_known_hosts_file,
            schema_scan_method,
            schema_scan_limit,
            schema_name,
            refresh_schema,
            log_level,
            log_path,
            retry_reads,
            read_preference,
            logging_timeout,
            replica_set,
            fetch_size }
];

// Data Source UI publishing description.
AmazonDocumentDBConnector.Publish = [
    Beta = true,
    Category = "Other",
    SupportsDirectQuery = true,

    ButtonText = { Extension.LoadString("ButtonTitle"), Extension.LoadString("ButtonHelp") },
    LearnMoreUrl = "https://aws.amazon.com/timestream/",

    SourceImage = AmazonDocumentDBConnector.Icons,
    SourceTypeImage = AmazonDocumentDBConnector.Icons
];

AmazonDocumentDBConnector.Icons = [
    Icon16 = { Extension.Contents("AmazonDocumentDBConnector16.png"), Extension.Contents("AmazonDocumentDBConnector20.png"), Extension.Contents("AmazonDocumentDBConnector24.png"), Extension.Contents("AmazonDocumentDBConnector32.png") },
    Icon32 = { Extension.Contents("AmazonDocumentDBConnector32.png"), Extension.Contents("AmazonDocumentDBConnector40.png"), Extension.Contents("AmazonDocumentDBConnector48.png"), Extension.Contents("AmazonDocumentDBConnector64.png") }
];

// Loads functions from another project file.
Extension.LoadFunction = (name as text) =>
    let
        binary = Extension.Contents(name),
        asText = Text.FromBinary(binary)
    in
        Expression.Evaluate(asText, #shared);

// Diagnostics module contains multiple functions.
Diagnostics = Extension.LoadFunction("Diagnostics.pqm");
Diagnostics.LogValue = if (EnableTraceOutput) then Diagnostics[LogValue] else (prefix, value) => value;

// OdbcConstants contains numeric constants from the ODBC header files, and helper function to create bitfield values.
ODBC = Extension.LoadFunction("OdbcConstants.pqm");
