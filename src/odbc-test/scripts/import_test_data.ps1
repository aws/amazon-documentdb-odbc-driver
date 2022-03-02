##################################################
# Assumptions
# 1. Docker container 'mongo' is running and instance of mongo server and listening on port 27017
# 2. Mongo started with the environment variables MONGO_INITDB_ROOT_USERNAME and MONGO_INITDB_ROOT_PASSWORD
##################################################

$TEST_INPUT_FOLDER = Join-Path -Path (Get-Item $PSCommandPath).DirectoryName  -ChildPath "..\input"
$CONTAINER_NAME  = if ( $env:CONTAINER_NAME ) { $env:CONTAINER_NAME } else { "mongo" }
$DATABASE_NAME   = "odbc-test"
$CONTAINER_INPUT_FOLDER = "/home/odbc-test"

# Copy test files to docker and drop database.
docker exec -i $CONTAINER_NAME bash -c "rm -fr $CONTAINER_INPUT_FOLDER"
docker exec -i $CONTAINER_NAME bash -c "mkdir -p $CONTAINER_INPUT_FOLDER"
docker cp $TEST_INPUT_FOLDER "$($CONTAINER_NAME):$($CONTAINER_INPUT_FOLDER)"
# Clear the database so we don't have any existing data
docker exec -i "$($CONTAINER_NAME)" `
    mongosh --quiet -u="$($env:MONGO_INITDB_ROOT_USERNAME)" -p="$($env:MONGO_INITDB_ROOT_PASSWORD)" --authenticationDatabase=admin `
        --eval "db.dropDatabase()" "$($DATABASE_NAME)"

# Load each test input file
Get-ChildItem -Path $TEST_INPUT_FOLDER -File -Filter *.json |
Foreach-Object {
    # Collection will be named the same as the input file base name
	$COLLECTION_NAME = $_.BaseName
	$TEST_FILE_NAME  = $_.Name

    # Log what input we're about to import
	Write-Output "docker exec -i $($CONTAINER_NAME) `
        mongoimport -u=$($env:MONGO_INITDB_ROOT_USERNAME) -p=... --authenticationDatabase=admin `
            -d=$($DATABASE_NAME) -c=$($COLLECTION_NAME) `
            --file=""$($CONTAINER_INPUT_FOLDER)/input/$($TEST_FILE_NAME)"""
    # Import the test input
	docker exec -i "$($CONTAINER_NAME)" `
        mongoimport --quiet -u="$($env:MONGO_INITDB_ROOT_USERNAME)" -p="$($env:MONGO_INITDB_ROOT_PASSWORD)" --authenticationDatabase=admin `
            -d="$($DATABASE_NAME)" -c="$($COLLECTION_NAME)" `
            --file="""$($CONTAINER_INPUT_FOLDER)/input/$($TEST_FILE_NAME)"""
    if (!$?) {
        # If error detected stop the rest of the processing and exit
        exit 1
    }
}
