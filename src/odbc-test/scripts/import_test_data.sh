#!/bin/bash

##################################################
# Assumptions
# 1. Mongo server running and listening on port 27017
# 2. Mongo started with the environment variables DOC_DB_USER_NAME and DOC_DB_PASSWORD
##################################################

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  SCRIPT_DIR=$( cd -- "$( dirname -- $0 )"  && pwd )
elif [[ "$OSTYPE" == "darwin"* ]]; then
  SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
fi

TEST_INPUT_FOLDER="${SCRIPT_DIR}/../input"
DATABASE_NAME="odbc-test"

# Clear the database so we don't have any existing data
mongosh --quiet -u="${DOC_DB_USER_NAME}" -p="${DOC_DB_PASSWORD}" --authenticationDatabase=admin \
    --eval "db.dropDatabase()" "${DATABASE_NAME}"

# Load each test input file
for FILENAME in ${TEST_INPUT_FOLDER}/*.json
do
    # Collection will be named the same as the input file base name
    COLLECTION_NAME="$(basename -- ${FILENAME%.json})"
    TEST_FILE_NAME="$(basename -- ${FILENAME})"

    mongoimport -u="${DOC_DB_USER_NAME}" -p="${DOC_DB_PASSWORD}" --authenticationDatabase=admin \
        -d="${DATABASE_NAME}" -c="${COLLECTION_NAME}" \
        --file="""${TEST_INPUT_FOLDER}/${TEST_FILE_NAME}"""
    if [ $? -ne 0 ]
    then
        exit 1
    fi
done
