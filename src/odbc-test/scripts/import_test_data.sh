#!/bin/sh

##################################################
# Assumptions
# 1. Docker container 'mongo' is running and instance of mongo server and listening on port 27017
# 2. Mongo started with the environment variables MONGO_INITDB_ROOT_USERNAME and MONGO_INITDB_ROOT_PASSWORD
##################################################

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
TEST_INPUT_FOLDER="${SCRIPT_DIR}/../input"
CONTAINER_NAME=${CONTAINER_NAME:-mongo}
DATABASE_NAME="odbc-test"
CONTAINER_INPUT_FOLDER="/home/odbc-test"

# Copy test files to docker and drop database.
docker exec -it ${CONTAINER_NAME} bash -c "rm -fr ${CONTAINER_INPUT_FOLDER}"
docker exec -it ${CONTAINER_NAME} bash -c "mkdir -p ${CONTAINER_INPUT_FOLDER}"
docker cp $TEST_INPUT_FOLDER "${CONTAINER_NAME}:${CONTAINER_INPUT_FOLDER}"
# Clear the database so we don't have any existing data
docker exec -it "${CONTAINER_NAME}" \
    mongosh --quiet -u="${MONGO_INITDB_ROOT_USERNAME}" -p="${MONGO_INITDB_ROOT_PASSWORD}" --authenticationDatabase=admin \
        --eval "db.dropDatabase()" "${DATABASE_NAME}"

# Load each test input file
for FILENAME in ${TEST_INPUT_FOLDER}/*.json
do
    # Collection will be named the same as the input file base name
    COLLECTION_NAME="$(basename -- ${FILENAME%.json})"
    TEST_FILE_NAME="$(basename -- ${FILENAME})"

    docker exec -it "${CONTAINER_NAME}" \
        mongoimport --quiet -u="${MONGO_INITDB_ROOT_USERNAME}" -p="${MONGO_INITDB_ROOT_PASSWORD}" --authenticationDatabase=admin \
            -d="${DATABASE_NAME}" -c="${COLLECTION_NAME}" \
            --file="""${CONTAINER_INPUT_FOLDER}/input/${TEST_FILE_NAME}"""
    if [ $? -ne 0 ]
    then
        exit 1
    fi
done
