// This file contains your Data Connector logic
[Version = "1.0.0"]
section AmazonDocumentDB;

// When set to true, additional trace information will be written out to the User log. 
// This should be set to false before release. Tracing is done through a call to 
// Diagnostics.LogValue(). When EnableTraceOutput is set to false, the call becomes a 
// no-op and simply returns the original value.
EnableTraceOutput = true;

Config_DriverName = "Amazon DocumentDB";

/* This is the method for connection to ODBC*/
[DataSource.Kind="AmazonDocumentDB", Publish="AmazonDocumentDB.UI"]

shared AmazonDocumentDB.Contents = Value.ReplaceType(AmazonDocumentDBImpl, AmazonDocumentDBType);

// Wrapper function to provide additional UI customization.
AmazonDocumentDBType = type function (
        dsn as (type text meta [
            Documentation.FieldCaption = "DSN",
            Documentation.FieldDescription = "DocumentDB DSN.",
            Documentation.SampleValues = { "Data Source Name" }
        ])
    )
    as table meta [
        Documentation.Name = "Amazon DocumentDB"
    ];

AmazonDocumentDBImpl = (dsn as text) as table =>
      let
        //
        // Connection string settings
        //
        ConnectionString = [
            DSN=dsn
        ],

        //
        // Handle credentials
        // Credentials are not persisted with the query and are set through a separate 
        // record field - CredentialConnectionString. The base Odbc.DataSource function
        // will handle UsernamePassword authentication automatically, but it is explictly
        // handled here as an example. 
        //
        Credential = Extension.CurrentCredential(),
        encryptionEnabled = Credential[EncryptConnection]? = true,
        CredentialConnectionString = [
            UID = Credential[Username],
            PWD = Credential[Password]
        ],
    
        MySqlCapabilities = Diagnostics.LogValue("SqlCapabilities_Options", [
            GroupByCapabilities = ODBC[SQL_GB][SQL_GB_GROUP_BY_CONTAINS_SELECT],
            SupportsNumericLiterals = true,
            SupportsStringLiterals = true,                
            SupportsOdbcDateLiterals = true,
            SupportsOdbcTimeLiterals = true,
            SupportsOdbcTimestampLiterals = true,
            PrepareStatements = false,
            StringLiteralEscapeCharacters = { "\" },
            SupportsTop = false,
            LimitClauseKind = LimitClauseKind.LimitOffset,
            FractionalSecondsScale = 3
        ]),
        MySQLGetInfo = Diagnostics.LogValue("SQLGetInfo_Options", [
            SQL_SQL92_PREDICATES = ODBC[SQL_SP][All], // (SQL_SP) all
            SQL_AGGREGATE_FUNCTIONS = 0xFF, //all
            SQL_SQL_CONFORMANCE = 0x8, // FULL
            SQL_GROUP_BY = ODBC[SQL_GB][SQL_GB_GROUP_BY_CONTAINS_SELECT]
        ]),
        MySQLGetFunctions = Diagnostics.LogValue("SQLGetFunctions_Options", [
            SQL_API_SQLBINDPARAMETER = false,
            SQL_CONVERT_FUNCTIONS = 0x2 // CAST
        ]),

        // Build AstVisitor
        // This record allows you to customize the generated SQL for certain
        // operations. The most common usage is to define syntax for LIMIT/OFFSET operators 
        // when TOP is not supported. 
        // 
        MyAstVisitor = [
            LimitClause = (skip, take) =>
            let
                offset = if (skip <> null and skip > 0) then Text.Format("OFFSET #{0} ROWS", {skip}) else "",
                limit = if (take <> null) then Text.Format("LIMIT #{0}", {take}) else ""
            in
                [
                Text = Text.Format("#{0} #{1}", {limit, offset}),
                Location = "AfterQuerySpecification"
                ]
            ],

        // SQLGetTypeInfo can be specified in two ways:
        // 1. A #table() value that returns the same type information as an ODBC
        //    call to SQLGetTypeInfo.
        // 2. A function that accepts a table argument, and returns a table. The 
        //    argument will contain the original results of the ODBC call to SQLGetTypeInfo.
        //    Your function implementation can modify/add to this table.
        //
        // For details of the format of the types table parameter and expected return value,
        // please see: https://docs.microsoft.com/en-us/sql/odbc/reference/syntax/sqlgettypeinfo-function
        //
        // The sample implementation provided here will simply output the original table
        // to the user trace log, without any modification. 
        SQLGetTypeInfo = (types) => 
            let 
                original =
                    if (EnableTraceOutput <> true) then
                        types 
                    else
                        let 
                            // Outputting the entire table might be too large, and result in the value being truncated.
                            // We can output a row at a time instead with Table.TransformRows()
                            rows = Table.TransformRows(types, each Diagnostics.LogValue("SQLGetTypeInfo " & _[TYPE_NAME], _)),
                            toTable = Table.FromRecords(rows)
                        in
                            toTable,
                modified = 
                    if (Table.SelectRows(original, each [TYPE_NAME] = "NVARCHAR") <> null) then 
                        let
                            // Fix NVARCHAR data type definition: the result  forces Mashup Engine to transform any NVARCHAR
                            // to LONGVARCHAR/VARCHAR/CHAR. Since the driver does not have definition to these types
                            // this will eliminate the necessity of the transformation
                            typesNVARCHARRemoved = Table.SelectRows(types, each not Text.Contains(_[TYPE_NAME], "NVARCHAR")),
                            nvarcharRecord = original{[TYPE_NAME="NVARCHAR"]} & [COLUMN_SIZE = 65532 ],
                            finalTable = typesNVARCHARRemoved & Table.FromRecords({nvarcharRecord})
                        in
                            finalTable
                    else
                        original
            in
                Value.ReplaceType(modified, Value.Type(types)),        

        //
        // Call to Odbc.DataSource
        //
        OdbcDatasource = Odbc.DataSource(ConnectionString, [
            // Enables client side connection pooling for the ODBC driver.
            // Most drivers will want to set this value to true.
            ClientConnectionPooling = true,
            // When HierarchialNavigation is set to true, the navigation tree
            // will be organized by Database -> Schema -> Table. When set to false,
            // all tables will be displayed in a flat list using fully qualified names. 
            HierarchicalNavigation = true,
            //Expose Native Query
            HideNativeQuery = false,
            //Allows the M engine to select a compatible data type when conversion between two specific numeric types is not declared as supported in the SQL_CONVERT_* capabilities.
            SoftNumbers = true,
            TolerateConcatOverflow = true,
            // These values should be set by previous steps
            CredentialConnectionString = CredentialConnectionString,
            //Requires AstVisitor since Top is not supported in DocumentDB
            AstVisitor = MyAstVisitor,
            SqlCapabilities = MySqlCapabilities,
            SQLGetInfo = MySQLGetInfo,
            SQLGetFunctions = MySQLGetFunctions,
            SQLGetTypeInfo = SQLGetTypeInfo
        ])
        
    in OdbcDatasource;


// Data Source Kind description
AmazonDocumentDB = [
 // Test Connection
    TestConnection = (dataSourcePath) => 
        let
            json = Json.Document(dataSourcePath),
            dsn = json[dsn]
        in
            { "AmazonDocumentDB.Contents", dsn}, 
 // Authentication Type
    Authentication = [
        UsernamePassword = []
    ],
    Label = Extension.LoadString("DataSourceLabel")
];

// Data Source UI publishing description
AmazonDocumentDB.UI = [
    Beta = true,
    Category = "Database",
    ButtonText = { Extension.LoadString("ButtonTitle"), Extension.LoadString("ButtonHelp") },
    LearnMoreUrl = "https://aws.amazon.com/documentdb/",
    SourceImage = AmazonDocumentDB.Icons,
    SourceTypeImage = AmazonDocumentDB.Icons,
    // This is for Direct Query Support
    SupportsDirectQuery = true
];

AmazonDocumentDB.Icons = [
    Icon16 = { Extension.Contents("AmazonDocumentDBConnector16.png"), Extension.Contents("AmazonDocumentDBConnector20.png"), Extension.Contents("AmazonDocumentDBConnector24.png"), Extension.Contents("AmazonDocumentDBConnector32.png") },
    Icon32 = { Extension.Contents("AmazonDocumentDBConnector32.png"), Extension.Contents("AmazonDocumentDBConnector40.png"), Extension.Contents("AmazonDocumentDBConnector48.png"), Extension.Contents("AmazonDocumentDBConnector64.png") }
];

// 
// Load common library functions
// 
Extension.LoadFunction = (name as text) =>
    let
        binary = Extension.Contents(name),
        asText = Text.FromBinary(binary)
    in
        Expression.Evaluate(asText, #shared);

// Diagnostics module contains multiple functions. We can take the ones we need.
Diagnostics = Extension.LoadFunction("Diagnostics.pqm");
Diagnostics.LogValue = if (EnableTraceOutput) then Diagnostics[LogValue] else (prefix, value) => value;

// OdbcConstants contains numeric constants from the ODBC header files, and a 
// helper function to create bitfield values.
ODBC = Extension.LoadFunction("OdbcConstants.pqm");
Odbc.Flags = ODBC[Flags];
