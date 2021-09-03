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

        ConnectionOptions = [
            // Connection string properties used for encrypted connections.
            CredentialConnectionString = EncryptedConnectionString
        ],

        OdbcDatasource = Odbc.DataSource(ConnectionString, ConnectionOptions)
    in
        OdbcDatasource;

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
