##################################################
# Assumptions
# 1. MongoDB server is running and listening on port 27017
# 2. Environment variables DOC_DB_USER_NAME and DOC_DB_PASSWORD already defined
##################################################

$TEST_INPUT_FOLDER = Join-Path -Path (Get-Item $PSCommandPath).DirectoryName  -ChildPath "..\input"
$DATABASE_NAME   = "odbc-test"

# Clear the database so we don't have any existing data
mongosh --quiet -u="$($env:DOC_DB_USER_NAME)" -p="$($env:DOC_DB_PASSWORD)" --authenticationDatabase=admin `
        --eval "db.dropDatabase()" "$($DATABASE_NAME)"

# Load each test input file
Get-ChildItem -Path $TEST_INPUT_FOLDER -File -Filter *.json |
Foreach-Object {
    # Collection will be named the same as the input file base name
    $COLLECTION_NAME = $_.BaseName
    $TEST_FILE_NAME  = $_.Name

    # Log what input we're about to import
    Write-Output `
        "mongoimport -u=$($env:DOC_DB_USER_NAME) -p=... --authenticationDatabase=admin `
            -d=$($DATABASE_NAME) -c=$($COLLECTION_NAME)  --jsonArray `
            --file=""$($TEST_INPUT_FOLDER)\$($TEST_FILE_NAME)"""
    # Import the test input
    if ($null -eq $env:MONGO_IMPORT_COMMAND) { 
        $env:MONGO_IMPORT_COMMAND = 'mongoimport' 
    }
    & $env:MONGO_IMPORT_COMMAND -u="$($env:DOC_DB_USER_NAME)" -p="$($env:DOC_DB_PASSWORD)" --authenticationDatabase=admin `
        -d="$($DATABASE_NAME)" -c="$($COLLECTION_NAME)" --jsonArray `
        --file="""$($TEST_INPUT_FOLDER)\$($TEST_FILE_NAME)"""
    if (!$?) {
        # If error detected stop the rest of the processing and exit
        exit 1
    }
}
