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

#ifndef _IGNITE_ODBC_JNI_DOCUMENTDB_CONNECTION_PROPERTIES
#define _IGNITE_ODBC_JNI_DOCUMENTDB_CONNECTION_PROPERTIES

using ignite::odbc::common::concurrent::SharedPointer;
using ignite::odbc::jni::ResultSet;
using ignite::odbc::jni::java::GlobalJObject;
using ignite::odbc::jni::java::JniContext;
using ignite::odbc::jni::java::JniErrorInfo;

namespace ignite {
namespace odbc {
namespace jni {
/**
 * Wrapper for the the DocumentDB connection properties.
 */
class DocumentDbConnectionProperties {
  friend class DocumentDbConnection;
  friend class DocumentDbQueryMappingService;

 public:
  ~DocumentDbConnectionProperties() = default;

 private:
  /**
   * Constructs an instance of the DocumentDbConnectionProperties class.
   */
  DocumentDbConnectionProperties(
      SharedPointer< JniContext >& jniContext,
      SharedPointer< GlobalJObject >& connectionProperties)
      : jniContext_(jniContext), connectionProperties_(connectionProperties) {
  }

  /** The JNI context */
  SharedPointer< JniContext > jniContext_;

  /** The DocumentDbConnectionProperties Java object  */
  SharedPointer< GlobalJObject > connectionProperties_;
};
}  // namespace jni
}  // namespace odbc
}  // namespace ignite

#endif  // _IGNITE_ODBC_JNI_DOCUMENTDB_CONNECTION_PROPERTIES
