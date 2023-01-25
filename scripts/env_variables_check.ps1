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

$scriptPath = Split-Path -parent $PSCommandPath
$projectPath = Split-Path -parent $scriptPath
$CHECK=1

if ( [string]::IsNullOrEmpty(${Env:DOC_DB_USER_NAME}) ) {
  Write-Output "DOC_DB_USER_NAME environment variable is not set. Using default."
  $Env:DOC_DB_USER_NAME = "documentdb"
  [System.Environment]::SetEnvironmentVariable('DOC_DB_USER_NAME', $Env:DOC_DB_USER_NAME, [System.EnvironmentVariableTarget]::User)
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
  $Env:DOCUMENTDB_HOME="${projectPath}\build\odbc\bin\Debug"
  [System.Environment]::SetEnvironmentVariable('DOCUMENTDB_HOME', $Env:DOCUMENTDB_HOME, [System.EnvironmentVariableTarget]::User)
  Write-Output "DOCUMENTDB_HOME=$Env:DOCUMENTDB_HOME"
} else {
  Write-Output "DOCUMENTDB_HOME=$Env:DOCUMENTDB_HOME"
}

if ( [string]::IsNullOrEmpty(${Env:VCPKG_ROOT}) ) {
  Write-Output "VCPKG_ROOT environment variable is not set. Using default."
  $Env:VCPKG_ROOT="C:\VCPKG"
  [System.Environment]::SetEnvironmentVariable('VCPKG_ROOT', $Env:VCPKG_ROOT, [System.EnvironmentVariableTarget]::User)
  Write-Output "VCPKG_ROOT=$Env:VCPKG_ROOT"
} else {
  Write-Output "VCPKG_ROOT=$Env:VCPKG_ROOT"
}

if ( "${CHECK}" -eq "0" ) {
  Write-Output "Missing envrionment variables, please set them accordingly."
} else {
  Write-Output "Environment variables are all set"
}
