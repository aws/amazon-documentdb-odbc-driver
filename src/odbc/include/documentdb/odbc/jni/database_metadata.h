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
#include <documentdb/odbc/jni/java.h>
#include <documentdb/odbc/jni/result_set.h>

#include <string>

#ifndef _DOCUMENTDB_ODBC_JNI_DATABASE_METADATA
#define _DOCUMENTDB_ODBC_JNI_DATABASE_METADATA

using documentdb::odbc::common::concurrent::SharedPointer;
using documentdb::odbc::jni::ResultSet;
using documentdb::odbc::jni::java::GlobalJObject;
using documentdb::odbc::jni::java::JniContext;
using documentdb::odbc::jni::java::JniErrorInfo;

namespace documentdb {
namespace odbc {
namespace jni {
/**
 * Wrapper for the the JDBC DatabaseMetaData.
 */
class DatabaseMetaData {
  friend class DocumentDbConnection;
  friend class DocumentDbQueryMappingService;

 public:
  ~DatabaseMetaData() = default;

  /**
   * Query the tables in the database according to the given search
   * critera in catalog (not supported), schemaPattern, tablePattern
   * and types of tables.
   */
  SharedPointer< ResultSet > GetTables(
      const boost::optional< std::string >& catalog,
      const boost::optional< std::string >& schemaPattern,
      const std::string& tableNamePattern,
      const boost::optional< std::vector< std::string > >& types,
      JniErrorInfo& errInfo);

  /**
   * Query the columns in the database according to the given
   * search critera in catalog (not supported), schemaPattern (nullable),
   * tablePattern and columnPattern.
   */
  SharedPointer< ResultSet > GetColumns(
      const boost::optional< std::string >& catalog,
      const boost::optional< std::string >& schemaPattern,
      const std::string& tableNamePattern, const std::string& columnNamePattern,
      JniErrorInfo& errInfo);

  /**
   * Query the primary keys in the database according to the given
   * search critera in catalog (not supported), schema, and
   * table.
   */
  SharedPointer< ResultSet > GetPrimaryKeys(
      const boost::optional< std::string >& catalog,
      const boost::optional< std::string >& schema,
      const boost::optional< std::string >& table, JniErrorInfo& errInfo);

  /**
   * Query the foreign keys in the database according to the given
   * search critera in catalog (not supported), schema, and
   * table.
   */
  SharedPointer< ResultSet > GetImportedKeys(
      const boost::optional< std::string >& catalog,
      const boost::optional< std::string >& schema, const std::string& table,
      JniErrorInfo& errInfo);

  /**
   * Query the type info in the database.
   */
  SharedPointer< ResultSet > GetTypeInfo(JniErrorInfo& errInfo);

 private:
  /**
   * Constructs an instance of the DatabaseMetaData class.
   */
  DatabaseMetaData(SharedPointer< JniContext >& jniContext,
                   SharedPointer< GlobalJObject >& databaseMetaData)
      : _jniContext(jniContext), _databaseMetaData(databaseMetaData) {
  }

  /** The JNI context */
  SharedPointer< JniContext > _jniContext;

  /** The DatabaseMetaData Java object  */
  SharedPointer< GlobalJObject > _databaseMetaData;
};
}  // namespace jni
}  // namespace odbc
}  // namespace documentdb

#endif  // _DOCUMENTDB_ODBC_JNI_DATABASE_METADATA
