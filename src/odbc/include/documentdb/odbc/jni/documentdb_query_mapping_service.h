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
#include <documentdb/odbc/jni/documentdb_connection_properties.h>
#include <documentdb/odbc/jni/documentdb_database_metadata.h>
#include <documentdb/odbc/jni/documentdb_mql_query_context.h>
#include <documentdb/odbc/jni/java.h>
#include <documentdb/odbc/jni/result_set.h>

#include <string>
#include <vector>

#ifndef _DOCUMENTDB_ODBC_JNI_DOCUMENTDB_QUERY_MAPPING_SERVICE
#define _DOCUMENTDB_ODBC_JNI_DOCUMENTDB_QUERY_MAPPING_SERVICE

using documentdb::odbc::common::concurrent::SharedPointer;
using documentdb::odbc::jni::ResultSet;
using documentdb::odbc::jni::java::GlobalJObject;
using documentdb::odbc::jni::java::JniContext;
using documentdb::odbc::jni::java::JniErrorInfo;

namespace documentdb {
namespace odbc {
namespace jni {
/**
 * Wrapper for the the DocumentDB query mapping service.
 */
class DocumentDbQueryMappingService {
 public:
  /**
   * Destructs DocumentDbQueryMappingService object.
   */
  ~DocumentDbQueryMappingService() = default;

  /**
   * Creates an instance of DocumentDbQueryMappingService from connection
   * properties and database metadata.
   *
   * @return an new instance of DocumentDbQueryMappingService.
   */
  static SharedPointer< DocumentDbQueryMappingService > Create(
      const SharedPointer< DocumentDbConnectionProperties >&
          connectionProperties,
      const SharedPointer< DocumentDbDatabaseMetadata >& databaseMetadata,
      JniErrorInfo& errInfo);

  /**
   * Getst the MQL query context for a SQL query.
   *
   * @return a pointer to DocumentDbMqlQueryContext.
   */
  SharedPointer< DocumentDbMqlQueryContext > GetMqlQueryContext(
      const std::string& sql, int maxRowCount, JniErrorInfo& errInfo);

 private:
  /**
   * Constructs an instance of the DocumentDbQueryMappingService class.
   */
  DocumentDbQueryMappingService(
      SharedPointer< JniContext >& jniContext,
      SharedPointer< GlobalJObject >& queryMappingService)
      : jniContext_(jniContext), queryMappingService_(queryMappingService) {
  }

  /**
   * Reads the JDBC column metadata into a vector of JdbcColumnMetadata.
   *
   * @return true if able to read the metadata, false, otherwise.
   */
  bool ReadJdbcColumnMetadata(
      SharedPointer< GlobalJObject > const& columnMetadata,
      std::vector< JdbcColumnMetadata >& columnMetadataList,
      JniErrorInfo& errInfo);

  /** The JNI context */
  SharedPointer< JniContext > jniContext_;

  /** The DocumentDbQueryMappingService Java object  */
  SharedPointer< GlobalJObject > queryMappingService_;
};
}  // namespace jni
}  // namespace odbc
}  // namespace documentdb

#endif  // _DOCUMENTDB_ODBC_JNI_DOCUMENTDB_QUERY_MAPPING_SERVICE
