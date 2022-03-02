##################################################
# Assumptions
# 1. Docker is available on the path.
# 2. Creating this dockker container with name "mongo" and port "27017"
# 3. If environment variables MONGO_INITDB_ROOT_USERNAME and MONGO_INITDB_ROOT_PASSWORD are defined, they will be used
##################################################

$CONTAINER_EXISTS = docker ps -q -f name=mongo -f status=exited -f status=running
echo "Container id: '$($CONTAINER_EXISTS)'"

if ( $CONTAINER_EXISTS  ) {
    docker start mongo
} else {
    $env:MONGO_INITDB_ROOT_USERNAME = if ( $env:MONGO_INITDB_ROOT_USERNAME ) { $env:MONGO_INITDB_ROOT_USERNAME } else { "adminuser" }
    $env:MONGO_INITDB_ROOT_PASSWORD = if ( $env:MONGO_INITDB_ROOT_PASSWORD ) { $env:MONGO_INITDB_ROOT_PASSWORD } else { [guid]::NewGuid().ToString() }

    docker run --name mongo -e MONGO_INITDB_ROOT_USERNAME -e MONGO_INITDB_ROOT_PASSWORD -d -p 27017:27017 mongo:latest
    if( !$? ) {
        exit 1
    }
}
