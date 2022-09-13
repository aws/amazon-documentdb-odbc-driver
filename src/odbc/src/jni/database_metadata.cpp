/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <documentdb/odbc/common/concurrent.h>
#include <documentdb/odbc/documentdb_error.h>
#include <documentdb/odbc/jni/database_metadata.h>
#include <documentdb/odbc/jni/java.h>

using documentdb::odbc::common::concurrent::SharedPointer;
using documentdb::odbc::jni::ResultSet;
using documentdb::odbc::jni::java::GlobalJObject;
using documentdb::odbc::jni::java::JniContext;
using documentdb::odbc::jni::java::JniErrorCode;
using documentdb::odbc::jni::java::JniErrorInfo;

namespace documentdb {
namespace odbc {
namespace jni {
SharedPointer< ResultSet > DatabaseMetaData::GetTables(
    const boost::optional< std::string >& catalog,
    const boost::optional< std::string >& schemaPattern,
    const std::string& tableNamePattern,
    const boost::optional< std::vector< std::string > >& types,
    JniErrorInfo& errInfo) {
  SharedPointer< GlobalJObject > resultSet;
  JniErrorCode success = _jniContext.Get()->DatabaseMetaDataGetTables(
      _databaseMetaData, catalog, schemaPattern, tableNamePattern, types,
      resultSet, errInfo);
  if (success != JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS) {
    return nullptr;
  }
  return new ResultSet(_jniContext, resultSet);
}

SharedPointer< ResultSet > DatabaseMetaData::GetColumns(
    const boost::optional< std::string >& catalog,
    const boost::optional< std::string >& schemaPattern,
    const std::string& tableNamePattern, const std::string& columnNamePattern,
    JniErrorInfo& errInfo) {
  SharedPointer< GlobalJObject > resultSet;
  const std::vector< std::string > types;
  JniErrorCode success = _jniContext.Get()->DatabaseMetaDataGetColumns(
      _databaseMetaData, catalog, schemaPattern, tableNamePattern,
      columnNamePattern, resultSet, errInfo);
  if (success != JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS) {
    return nullptr;
  }
  return new ResultSet(_jniContext, resultSet);
}

SharedPointer< ResultSet > DatabaseMetaData::GetPrimaryKeys(
    const boost::optional< std::string >& catalog,
    const boost::optional< std::string >& schema,
    const boost::optional< std::string >& table, JniErrorInfo& errInfo) {
  SharedPointer< GlobalJObject > resultSet;
  const std::vector< std::string > types;
  JniErrorCode success = _jniContext.Get()->DatabaseMetaDataGetPrimaryKeys(
      _databaseMetaData, catalog, schema, table, resultSet, errInfo);
  if (success != JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS) {
    return nullptr;
  }
  return new ResultSet(_jniContext, resultSet);
}

SharedPointer< ResultSet > DatabaseMetaData::GetImportedKeys(
    const boost::optional< std::string >& catalog,
    const boost::optional< std::string >& schema, const std::string& table,
    JniErrorInfo& errInfo) {
  SharedPointer< GlobalJObject > resultSet;
  const std::vector< std::string > types;
  JniErrorCode success = _jniContext.Get()->DatabaseMetaDataGetImportedKeys(
      _databaseMetaData, catalog, schema, table, resultSet, errInfo);
  if (success != JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS) {
    return nullptr;
  }
  return new ResultSet(_jniContext, resultSet);
}

SharedPointer< ResultSet > DatabaseMetaData::GetTypeInfo(
    JniErrorInfo& errInfo) {
  SharedPointer< GlobalJObject > resultSet;
  const std::vector< std::string > types;
  JniErrorCode success = _jniContext.Get()->DatabaseMetaDataGetTypeInfo(
      _databaseMetaData, resultSet, errInfo);
  if (success != JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS) {
    return nullptr;
  }
  return new ResultSet(_jniContext, resultSet);
}
}  // namespace jni
}  // namespace odbc
}  // namespace documentdb
