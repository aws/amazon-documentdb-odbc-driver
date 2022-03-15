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
#include <ignite/odbc/jni/java.h>
#include <ignite/odbc/jni/result_set.h>

#include <string>

#ifndef _IGNITE_ODBC_JNI_DOCUMENTDB_DATABASE_METADATA
#define _IGNITE_ODBC_JNI_DOCUMENTDB_DATABASE_METADATA

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
class DocumentDbDatabaseMetadata {
  friend class DocumentDbConnection;
  friend class DocumentDbQueryMappingService;

 public:
  /**
   * Desctructs a DocumentDbDatabaseMetadata instance.
   */
  ~DocumentDbDatabaseMetadata() = default;

 private:
  /**
   * Constructs an instance of the DatabaseMetaData class.
   */
  DocumentDbDatabaseMetadata(
      SharedPointer< JniContext >& jniContext,
      SharedPointer< GlobalJObject >& databaseMetadata)
      : _jniContext(jniContext), _databaseMetadata(databaseMetadata) {
  }

  /** The JNI context */
  SharedPointer< JniContext > _jniContext;

  /** The DatabaseMetaData Java object  */
  SharedPointer< GlobalJObject > _databaseMetadata;
};
}  // namespace jni
}  // namespace odbc
}  // namespace ignite

#endif  // _IGNITE_ODBC_JNI_DOCUMENTDB_DATABASE_METADATA
