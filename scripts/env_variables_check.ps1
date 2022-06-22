
$scriptPath = $PSCommandPath

CHECK=1

if ( [string]::IsNullOrEmpty(${Env:DOC_DB_USER_NAME}) ) then
  Write-Output "DOC_DB_USER_NAME environment variable is not set. Using default."
  $Env:DOC_DB_USER_NAME=documentdb
  Write-Output "DOC_DB_USER_NAME=$DOC_DB_USER_NAME"
else
  Write-Output "DOC_DB_USER_NAME=$DOC_DB_USER_NAME"
fi

if ( [string]::IsNullOrEmpty(${Env:DOC_DB_PASSWORD}) ) then
  Write-Output "DOC_DB_PASSWORD environment variable is not set. Please set this variable explicitly."
  CHECK=0
else
  Write-Output "DOC_DB_PASSWORD=******"
fi

if ( [string]::IsNullOrEmpty(${Env:DOC_DB_USER}) ) then
  Write-Output "DOC_DB_USER environment variable is not set. Please set this variable explicitly."
  CHECK=0
else
  Write-Output "DOC_DB_USER=$DOC_DB_USER"
fi

if ( [string]::IsNullOrEmpty(${Env:DOC_DB_LOCAL_PORT}) ) then
  Write-Output "DOC_DB_LOCAL_PORT environment variable is not set. Please set this variable explicitly."
  CHECK=0
else
  Write-Output "DOC_DB_LOCAL_PORT=$DOC_DB_LOCAL_PORT"
fi

if ( [string]::IsNullOrEmpty(${Env:DOC_DB_PRIV_KEY_FILE}) ) then
  Write-Output "DOC_DB_PRIV_KEY_FILE environment variable is not set. Please set this variable explicitly."
  CHECK=0
else 
  Write-Output "DOC_DB_PRIV_KEY_FILE$DOC_DB_PRIV_KEY_FILE"
fi

if ( [string]::IsNullOrEmpty(${Env:JAVA_HOME}) ) then
  Write-Output "JAVA_HOME environment variable is not set. Please set this variable explicitly."
  CHECK=0
else
  Write-Output "JAVA_HOME=$JAVA_HOME"
fi

if ( [string]::IsNullOrEmpty(${Env:DOCUMENTDB_HOME}) ) then
  Write-Output "DOCUMENTDB_HOME environment variable is not set. Using default."
  $Env:DOCUMENTDB_HOME=${scriptPath}/../build/odbc/bin
  Write-Output "DOCUMENTDB_HOME=$DOCUMENTDB_HOME"
else
  Write-Output "DOCUMENTDB_HOME=$DOCUMENTDB_HOME"
fi

if ( [string]::IsNullOrEmpty(${Env:ODBC_LIB_PATH}) ) then
  Write-Output "ODBC_LIB_PATH environment variable is not set. Using default."
  $Env:ODBC_LIB_PATH=${scriptPath}/../build/odbc/lib
  Write-Output "ODBC_LIB_PATH=$ODBC_LIB_PATH"
else
  Write-Output "ODBC_LIB_PATH=$ODBC_LIB_PATH"
fi

if ( "${Env:CHECK}" -eq "0" ) then
  Write-Output "Missing envrionment variables, please set them accordingly."
else
  Write-Output "Environment variables are all set"
fi
