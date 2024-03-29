#!/bin/sh

##################################################
# Assumptions
# 1. Environment variables DOC_DB_USER_NAME and DOC_DB_PASSWORD must be defined.
# 2. This is running on MacOS Intel silicon
##################################################

# Get the path to the configuration file (Intel silicon)
MONGO_CONFIG_FILE=/usr/local/etc/mongod.conf
MONGODB_SERVICE_NAME="mongodb-community"

function update_mongod_config_authorization() {
    MONGO_ENABLED_DISABLED=${1:-enabled}
    if [ -f "$MONGO_CONFIG_FILE" ]
    then
        MONGO_AUTHORIZATION=$(grep -E '^[ \t]*authorization:[ \t]+["]?((enabled)?|(disabled)?)["]?' "${MONGO_CONFIG_FILE}")
        if [ -z "${MONGO_AUTHORIZATION}" ]
        then
            echo 'security:' >> "${MONGO_CONFIG_FILE}"
            echo "    authorization: \"${MONGO_ENABLED_DISABLED}\"" >> "${MONGO_CONFIG_FILE}"
        else
            echo "Updating in place...."
            cat "${MONGO_CONFIG_FILE}"
            sed "s/^[ \t]*authorization:.*$/    authorization: \"${MONGO_ENABLED_DISABLED}\"/g" "${MONGO_CONFIG_FILE}" > "${MONGO_CONFIG_FILE}.new"
            mv -f "${MONGO_CONFIG_FILE}.new" "${MONGO_CONFIG_FILE}"
            echo "Updated"
            cat "${MONGO_CONFIG_FILE}"
        fi
    else 
        echo "Config file not found: \"${MONGO_CONFIG_FILE}\""
    fi
}

brew tap mongodb/brew
brew install ${MONGODB_SERVICE_NAME}

update_mongod_config_authorization "disabled"
brew services restart ${MONGODB_SERVICE_NAME}

# By reinstalling, we've disabled authorization - so we can reset the authorization from scratch.
mongosh --eval "db.dropAllUsers()" "admin"
mongosh --eval "db.createUser({ user: '${DOC_DB_USER_NAME}', pwd: '${DOC_DB_PASSWORD}', roles: [ { role: 'root', db: 'admin' } ] })" "admin"
mongosh --eval "db.createUser({ user: 'docDbRestricted', pwd: '${DOC_DB_PASSWORD}', roles: [ { role: 'readAnyDatabase', db: 'admin' } ] })" "admin"

update_mongod_config_authorization "enabled"
brew services restart ${MONGODB_SERVICE_NAME}
