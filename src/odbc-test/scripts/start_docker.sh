#!/bin/sh

##################################################
# Assumptions
# 1. Docker is available on the path.
# 2. Creating this dockker container with name "mongo" and port "27017"
# 3. If environment variables MONGO_INITDB_ROOT_USERNAME and MONGO_INITDB_ROOT_PASSWORD are defined, they will be used
##################################################

CONTAINER_EXISTS=$(docker ps -q -f name=mongo -f status=exited -f status=running)
echo "Container id: '${CONTAINER_EXISTS}'"

if [ -z ${CONTAINER_EXISTS} ]
then
    export MONGO_INITDB_ROOT_USERNAME=${MONGO_INITDB_ROOT_USERNAME:-adminuser}
    export MONGO_INITDB_ROOT_PASSWORD=${MONGO_INITDB_ROOT_PASSWORD:-$(echo $RANDOM | md5sum | head -c 20)}

    docker run --name mongo -e MONGO_INITDB_ROOT_USERNAME -e MONGO_INITDB_ROOT_PASSWORD -d -p 27017:27017 mongo:latest
    if [ $? -ne 0 ]
    then
        exit 1
    fi
else
    docker start mongo
fi
