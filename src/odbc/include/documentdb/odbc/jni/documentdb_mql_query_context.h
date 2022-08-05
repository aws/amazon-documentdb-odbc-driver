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
#include <documentdb/odbc/jni/jdbc_column_metadata.h>
#include <documentdb/odbc/jni/result_set.h>

#include <map>
#include <string>
#include <vector>

#ifndef _DOCUMENTDB_ODBC_JNI_DOCUMENTDB_MQL_QUERY_CONTEXT
#define _DOCUMENTDB_ODBC_JNI_DOCUMENTDB_MQL_QUERY_CONTEXT

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
class DocumentDbMqlQueryContext {
  friend class DocumentDbConnection;
  friend class DocumentDbQueryMappingService;

 public:
  /**
   * Destructs a DocumentDbMqlQueryContext object.
   */
  ~DocumentDbMqlQueryContext() = default;

  /**
   * Gets the collection name.
   */
  const std::string GetCollectionName() {
    return _collectionName;
  }

  /**
   * Gets the list of aggregate operations
   */
  std::vector< std::string >& GetAggregateOperations() {
    return _aggregateOperations;
  }

  /**
   * Gets the column metadata for the query.
   */
  std::vector< JdbcColumnMetadata >& GetColumnMetadata() {
    return _columnMetadata;
  }

  /**
   * Gets the DocumentDB paths in the document for each of the columns.
   */
  std::vector< std::string >& GetPaths() {
    return _paths;
  }

 private:
  /**
   * Constructs an instance of the DatabaseMetaData class.
   */
  DocumentDbMqlQueryContext(const std::string& collectionName)
      : _collectionName(collectionName) {
    // No-opt
  }

  /**
   * The collection name.
   */
  const std::string _collectionName;

  /**
   * The aggregate operations.
   */
  std::vector< std::string > _aggregateOperations;

  /**
   * The column metadata.
   */
  std::vector< JdbcColumnMetadata > _columnMetadata;

  /**
   * The DocumentDB paths in the document for each column.
   */
  std::vector< std::string > _paths;
};
}  // namespace jni
}  // namespace odbc
}  // namespace documentdb

#endif  // _DOCUMENTDB_ODBC_JNI_DOCUMENTDB_MQL_QUERY_CONTEXT
