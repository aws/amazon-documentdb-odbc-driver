CHECK=1
if [[ -z "${DOC_DB_USER_NAME}" ]]; then
  echo "DOC_DB_USER_NAME environment variable is not set"
  CHECK=0
else
  echo $DOC_DB_USER_NAME
fi
if [[ -z "${DOC_DB_PASSWORD}" ]]; then
  echo "DOC_DB_PASSWORD environment variable is not set"
  CHECK=0
else
  echo $DOC_DB_PASSWORD
fi
if [[ -z "${DOC_DB_USER}" ]]; then
  echo "DOC_DB_USER environment variable is not set"
  CHECK=0
else
  echo $DOC_DB_USER
fi
if [[ -z "${DOC_DB_LOCAL_PORT}" ]]; then
  echo "DOC_DB_LOCAL_PORT environment variable is not set"
  CHECK=0
else
  echo $DOC_DB_LOCAL_PORT
fi
if [[ -z "${DOC_DB_PRIV_KEY_FILE}" ]]; then
  echo "DOC_DB_PRIV_KEY_FILE environment variable is not set"
  CHECK=0
else 
  echo $DOC_DB_PRIV_KEY_FILE
fi
if [[ -z "${JAVA_HOME}" ]]; then
  echo "JAVA_HOME environment variable is not set"
  CHECK=0
else
  echo $JAVA_HOME
fi
if [[ -z "${DOCUMENTDB_HOME}" ]]; then
  echo "DOCUMENTDB_HOME environment variable is not set"
  CHECK=0
else
  echo $DOCUMENTDB_HOME
fi
if [[ -z "${ODBC_LIB_PATH}" ]]; then
  echo "ODBC_LIB_PATH environment variable is not set"
  CHECK=0
else
  echo $ODBC_LIB_PATH
fi

if [[ "${CHECK}" == 0]]; then
  echo "Missing envrionment variables, please set them accordingly."
else
  echo "Environment variables are all set"
fi
