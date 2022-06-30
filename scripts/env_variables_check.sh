#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
SCRIPT_DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"

CHECK=1

if [[ -z "${DOC_DB_USER_NAME}" ]]; then
  echo "DOC_DB_USER_NAME environment variable is not set. Using default."
  export DOC_DB_USER_NAME=documentdb
  echo "DOC_DB_USER_NAME=$DOC_DB_USER_NAME"
else
  echo "DOC_DB_USER_NAME=$DOC_DB_USER_NAME"
fi

if [[ -z "${DOC_DB_PASSWORD}" ]]; then
  echo "DOC_DB_PASSWORD environment variable is not set. Please set this variable explicitly."
  CHECK=0
else
  echo "DOC_DB_PASSWORD=******"
fi

if [[ -z "${DOC_DB_USER}" ]]; then
  echo "DOC_DB_USER environment variable is not set. Please set this variable explicitly."
  CHECK=0
else
  echo "DOC_DB_USER=$DOC_DB_USER"
fi

if [[ -z "${DOC_DB_LOCAL_PORT}" ]]; then
  echo "DOC_DB_LOCAL_PORT environment variable is not set. Please set this variable explicitly."
  CHECK=0
else
  echo "DOC_DB_LOCAL_PORT=$DOC_DB_LOCAL_PORT"
fi

if [[ -z "${DOC_DB_PRIV_KEY_FILE}" ]]; then
  echo "DOC_DB_PRIV_KEY_FILE environment variable is not set. Please set this variable explicitly."
  CHECK=0
else 
  echo "DOC_DB_PRIV_KEY_FILE$DOC_DB_PRIV_KEY_FILE"
fi

if [[ -z "${JAVA_HOME}" ]]; then
  echo "JAVA_HOME environment variable is not set. Please set this variable explicitly."
  CHECK=0
else
  echo "JAVA_HOME=$JAVA_HOME"
fi

if [[ -z "${DOCUMENTDB_HOME}" ]]; then
  echo "DOCUMENTDB_HOME environment variable is not set. Using default."
  export DOCUMENTDB_HOME=${SCRIPT_DIR}/../build/odbc/bin
  echo "DOCUMENTDB_HOME=$DOCUMENTDB_HOME"
else
  echo "DOCUMENTDB_HOME=$DOCUMENTDB_HOME"
fi

if [[ -z "${DOCUMENTDB_HOME}" ]]; then
  echo "DOCUMENTDB_HOME environment variable is not set. Using default."
  export DOCUMENTDB_HOME=${SCRIPT_DIR}/../build/odbc/bin
  echo "DOCUMENTDB_HOME=$DOCUMENTDB_HOME"
else
  echo "DOCUMENTDB_HOME=$DOCUMENTDB_HOME"
fi

if [[ -z "${ODBCINSTINI}" ]]; then
  echo "ODBCINSTINI environment variable is not set. Using default."
  export ODBCINSTINI="${SCRIPT_DIR}/../build/odbc/lib/ignite-odbc-install.ini"
  echo "ODBCINSTINI=ODBCINSTINI"
else
  echo "ODBCINSTINI=ODBCINSTINI"
fi

#if [[ -z "${ODBC_LIB_PATH}" ]]; then
#  echo "ODBC_LIB_PATH environment variable is not set. Using default."
#  export ODBC_LIB_PATH=${SCRIPT_DIR}/../build/odbc/lib
#  echo "ODBC_LIB_PATH=$ODBC_LIB_PATH"
#else
#  echo "ODBC_LIB_PATH=$ODBC_LIB_PATH"
#fi

if [[ "${CHECK}" -eq "0" ]]; then
  echo "Missing envrionment variables, please set them accordingly."
else
  echo "Environment variables are all set"
fi
