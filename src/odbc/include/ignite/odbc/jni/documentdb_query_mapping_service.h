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

#include <ignite/odbc/common/concurrent.h>
#include <ignite/odbc/jni/documentdb_connection_properties.h>
#include <ignite/odbc/jni/documentdb_database_metadata.h>
#include <ignite/odbc/jni/documentdb_mql_query_context.h>
#include <ignite/odbc/jni/java.h>
#include <ignite/odbc/jni/result_set.h>

#include <string>
#include <vector>

#ifndef _IGNITE_ODBC_JNI_DOCUMENTDB_QUERY_MAPPING_SERVICE
#define _IGNITE_ODBC_JNI_DOCUMENTDB_QUERY_MAPPING_SERVICE

using ignite::odbc::common::concurrent::SharedPointer;
using ignite::odbc::jni::ResultSet;
using ignite::odbc::jni::java::GlobalJObject;
using ignite::odbc::jni::java::JniContext;
using ignite::odbc::jni::java::JniErrorInfo;

namespace ignite {
namespace odbc {
namespace jni {
/**
 * Wrapper for the the JDBC DatabaseMetaData.
 */
class DocumentDbQueryMappingService {
 public:
  /**
   * Destructs DocumentDbQueryMappingService object.
   */
  ~DocumentDbQueryMappingService() = default;

  /** 
   * Creates an instance of DocumentDbQueryMappingService from connection properties and database metadata.
   * 
   * @return an new instance of DocumentDbQueryMappingService.
   */
  static SharedPointer< DocumentDbQueryMappingService > Create(
      const SharedPointer< DocumentDbConnectionProperties >& connectionProperties,
      const SharedPointer< DocumentDbDatabaseMetadata >& databaseMetadata, JniErrorInfo& errInfo);

  /** 
   * Getst the MQL query context for a SQL query.
   * 
   * @return a pointer to DocumentDbMqlQueryContext.
   */
  SharedPointer< DocumentDbMqlQueryContext > GetMqlQueryContext(
      const std::string& sql, int maxRowCount, JniErrorInfo& errInfo);

 private:
  /**
   * Constructs an instance of the DatabaseMetaData class.
   */
  DocumentDbQueryMappingService(SharedPointer< JniContext >& jniContext,
                            SharedPointer< GlobalJObject >& queryMappingService)
      : _jniContext(jniContext), _queryMappingService(queryMappingService) {
  }

  /**
   * Reads the JDBC column metadata into a vector of JdbcColumnMetadata.
   * 
   * @return true if able to read the metadata, false, otherwise.
   */
  bool DocumentDbQueryMappingService::ReadJdbcColumnMetadata(
      SharedPointer< GlobalJObject > const& columnMetadata, 
      std::vector< JdbcColumnMetadata >& columnMetadataList,
      JniErrorInfo& errInfo);

  /** The JNI context */
  SharedPointer< JniContext > _jniContext;

  /** The DatabaseMetaData Java object  */
  SharedPointer< GlobalJObject > _queryMappingService;
};
}  // namespace jni
}  // namespace odbc
}  // namespace ignite

#endif  // _IGNITE_ODBC_JNI_DOCUMENTDB_QUERY_MAPPING_SERVICE
