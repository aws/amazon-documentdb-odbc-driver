
$scriptPath = Split-Path -parent $PSCommandPath

$CHECK=1

if ( [string]::IsNullOrEmpty(${Env:DOC_DB_USER_NAME}) ) {
  Write-Output "DOC_DB_USER_NAME environment variable is not set. Using default."
  $Env:DOC_DB_USER_NAME=documentdb
  Write-Output "DOC_DB_USER_NAME=$Env:DOC_DB_USER_NAME"
} else {
  Write-Output "DOC_DB_USER_NAME=$Env:DOC_DB_USER_NAME"
}

if ( [string]::IsNullOrEmpty(${Env:DOC_DB_PASSWORD}) ) {
  Write-Output "DOC_DB_PASSWORD environment variable is not set. Please set this variable explicitly."
  $CHECK=0
} else {
  Write-Output "DOC_DB_PASSWORD=******"
}

if ( [string]::IsNullOrEmpty(${Env:DOC_DB_USER}) ) {
  Write-Output "DOC_DB_USER environment variable is not set. Please set this variable explicitly."
  $CHECK=0
} else {
  Write-Output "DOC_DB_USER=$Env:DOC_DB_USER"
}

if ( [string]::IsNullOrEmpty(${Env:DOC_DB_LOCAL_PORT}) ) {
  Write-Output "DOC_DB_LOCAL_PORT environment variable is not set. Please set this variable explicitly."
  $CHECK=0
} else {
  Write-Output "DOC_DB_LOCAL_PORT=$Env:DOC_DB_LOCAL_PORT"
}

if ( [string]::IsNullOrEmpty(${Env:DOC_DB_PRIV_KEY_FILE}) ) {
  Write-Output "DOC_DB_PRIV_KEY_FILE environment variable is not set. Please set this variable explicitly."
  $CHECK=0
} else { 
  Write-Output "DOC_DB_PRIV_KEY_FILE=$Env:DOC_DB_PRIV_KEY_FILE"
}

if ( [string]::IsNullOrEmpty(${Env:JAVA_HOME}) ) {
  Write-Output "JAVA_HOME environment variable is not set. Please set this variable explicitly."
  $CHECK=0
} else {
  Write-Output "JAVA_HOME=$Env:JAVA_HOME"
}

if ( [string]::IsNullOrEmpty(${Env:DOCUMENTDB_HOME}) ) {
  Write-Output "DOCUMENTDB_HOME environment variable is not set. Using default."
  $Env:DOCUMENTDB_HOME="${scriptPath}\..\build\odbc\bin\Debug"
  Write-Output "DOCUMENTDB_HOME=$Env:DOCUMENTDB_HOME"
} else {
  Write-Output "DOCUMENTDB_HOME=$Env:DOCUMENTDB_HOME"
}

#ODBCINSTINI

#if ( [string]::IsNullOrEmpty(${Env:ODBC_LIB_PATH}) ) {
#  Write-Output "ODBC_LIB_PATH environment variable is not set. Using default."
#  $Env:ODBC_LIB_PATH="${scriptPath}\..\build\odbc\lib"
#  Write-Output "ODBC_LIB_PATH=$Env:ODBC_LIB_PATH"
#} else {
#  Write-Output "ODBC_LIB_PATH=$Env:ODBC_LIB_PATH"
#}

if ( "${CHECK}" -eq "0" ) {
  Write-Output "Missing envrionment variables, please set them accordingly."
} else {
  Write-Output "Environment variables are all set"
}
