section TimestreamConnector;

// When set to true, additional trace information will be written out to the User log. 
// This should be set to false before release. Tracing is done through a call to 
// Diagnostics.LogValue(). When EnableTraceOutput is set to false, the call becomes a 
// no-op and simply returns the original value.
EnableTraceOutput = true;

[DataSource.Kind="TimestreamConnector", Publish="TimestreamConnector.Publish"]
shared TimestreamConnector.Contents = Value.ReplaceType(TimestreamConnectorImpl, TimestreamConnectorType);

// Wrapper function to provide additional UI customization.
TimestreamConnectorType = type function (
        DSN as (type text meta [
            Documentation.FieldCaption = "DSN",
            Documentation.FieldDescription = "The DSN for Timestream.",
            Documentation.SampleValues = { "testdsn" }
        ])
    )
    as table meta [
        Documentation.Name = "Timestream"
    ];

TimestreamConnectorImpl = (DSN as text) as table =>
    let
        Credential = Extension.CurrentCredential(),

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

        ConnectionString = [
            Driver = "Amazon Timestream ODBC Driver",
            DSN = DSN
        ],

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

        ConnectionOptions = [
            // Handlers for ODBC driver capabilities.
            SqlCapabilities = SqlCapabilities,
            SQLColumns = SQLColumns,
            SQLGetInfo = SQLGetInfo,
            SQLGetFunctions = SQLGetFunctions,
            SQLGetTypeInfo = SQLGetTypeInfo,
            
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
        
        // Failed to connect to given host
        IsHostUnreachable = (OdbcErrorCode = 202)
    in
        if IsDriverNotInstalled then
            error Error.Record(errorRecord[Reason], "The Timestream ODBC driver is not installed. Please install the driver.", errorRecord[Detail])
        else if IsHostUnreachable then
            error Error.Record(errorRecord[Reason], "Could not reach server. Please double-check the authentication.", errorRecord[Detail])
        else 
            error errorRecord;

// Data Source Kind description.
TimestreamConnector = [
    Label = Extension.LoadString("DataSourceLabel"),

    SupportsEncryption = true,
    Authentication = [
        Implicit = []
    ],
    
    // Needed for use with Power BI Service.
    TestConnection = (dataSourcePath) => 
        let
            json = Json.Document(dataSourcePath),
            DSN = json[DSN]
        in
            { "TimestreamConnector.Contents", DSN }
];

// Data Source UI publishing description.
TimestreamConnector.Publish = [
    Beta = true,
    Category = "Other",

    ButtonText = { Extension.LoadString("ButtonTitle"), Extension.LoadString("ButtonHelp") },
    LearnMoreUrl = "https://aws.amazon.com/timestream/",

    SourceImage = TimestreamConnector.Icons,
    SourceTypeImage = TimestreamConnector.Icons
];

TimestreamConnector.Icons = [
    Icon16 = { Extension.Contents("Timestream16.png"), Extension.Contents("Timestream20.png"), Extension.Contents("Timestream24.png"), Extension.Contents("Timestream32.png") },
    Icon32 = { Extension.Contents("Timestream32.png"), Extension.Contents("Timestream40.png"), Extension.Contents("Timestream48.png"), Extension.Contents("Timestream64.png") }
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
