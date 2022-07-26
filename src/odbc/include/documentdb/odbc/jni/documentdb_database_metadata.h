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

#ifndef _DOCUMENTDB_ODBC_JNI_DOCUMENTDB_DATABASE_METADATA
#define _DOCUMENTDB_ODBC_JNI_DOCUMENTDB_DATABASE_METADATA

using documentdb::odbc::common::concurrent::SharedPointer;
using documentdb::odbc::jni::ResultSet;
using documentdb::odbc::jni::java::GlobalJObject;
using documentdb::odbc::jni::java::JniContext;
using documentdb::odbc::jni::java::JniErrorInfo;

namespace documentdb {
namespace odbc {
namespace jni {
/**
 * Wrapper for the the DocumentDB database metadata.
 */
class DocumentDbDatabaseMetadata {
  friend class DocumentDbConnection;
  friend class DocumentDbQueryMappingService;

 public:
  /**
   * Destructs a DocumentDbDatabaseMetadata instance.
   */
  ~DocumentDbDatabaseMetadata() = default;

 private:
  /**
   * Constructs an instance of the DocumentDbDatabaseMetadata class.
   */
  DocumentDbDatabaseMetadata(SharedPointer< JniContext >& jniContext,
                             SharedPointer< GlobalJObject >& databaseMetadata)
      : jniContext_(jniContext), databaseMetadata_(databaseMetadata) {
  }

  /** The JNI context */
  SharedPointer< JniContext > jniContext_;

  /** The DocumentDbDatabaseMetadata Java object  */
  SharedPointer< GlobalJObject > databaseMetadata_;
};
}  // namespace jni
}  // namespace odbc
}  // namespace documentdb

#endif  // _DOCUMENTDB_ODBC_JNI_DOCUMENTDB_DATABASE_METADATA
