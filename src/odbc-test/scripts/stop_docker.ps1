##################################################
# Assumptions
# 1. Docker is available on the path.
# 2. Creating this dockker container with name "mongo" and port "27017"
# 3. If environment variables MONGO_INITDB_ROOT_USERNAME and MONGO_INITDB_ROOT_PASSWORD are defined, they will be used
##################################################
param ( [switch] $Remove )
docker stop mongo
if( !$? ) {
    exit 1
}

if( $Remove.IsPresent ) {
    docker rm mongo
}
if( !$? ) {
    exit 1
}
