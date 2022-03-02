#!/bin/sh

##################################################
# Assumptions
# 1. Docker is available on the path.
# 2. Creating this dockker container with name "mongo" and port "27017"
# 3. If environment variables MONGO_INITDB_ROOT_USERNAME and MONGO_INITDB_ROOT_PASSWORD are defined, they will be used
##################################################

REMOVE=${1:-false}

docker stop mongo
if [ $? -ne 0 ]; then
    exit 1
fi;

if [  $REMOVE -ne "false" ]
then
    docker rm mongo
    if [ $? -ne 0 ]
    then
        exit 1
    fi
fi
