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
#include <ignite/odbc/config/configuration.h>
#include <ignite/odbc/jni/java.h>
#include <ignite/odbc/jni/database_metadata.h>

#ifndef _IGNITE_ODBC_JNI_DOCUMENTDB_CONNECTION
#define _IGNITE_ODBC_JNI_DOCUMENTDB_CONNECTION

using ignite::odbc::common::concurrent::SharedPointer;
using ignite::odbc::config::Configuration;
using ignite::odbc::jni::java::JniContext;
using ignite::odbc::jni::java::GlobalJObject;
using ignite::odbc::jni::java::JniErrorInfo;

namespace ignite {
    namespace odbc {
        namespace jni {
            class DocumentDbConnection {
               public:
                DocumentDbConnection(SharedPointer< JniContext > jniContext)
                    : _jniContext(jniContext) {
                }

                ~DocumentDbConnection();

                bool Open(const Configuration& config, JniErrorInfo& errInfo);

                bool Close(JniErrorInfo& errInfo);

                bool IsOpen() {
                    return _connection.IsValid();
                }

                SharedPointer< DatabaseMetaData > GetMetaData(
                    JniErrorInfo& errInfo);

               private:
                SharedPointer< JniContext > GetJniContext() {
                    return _jniContext;
                };
                static std::string FormatJdbcConnectionString(
                    const config::Configuration& config);

                SharedPointer< JniContext > _jniContext;
                SharedPointer< GlobalJObject > _connection;

                IGNITE_NO_COPY_ASSIGNMENT(DocumentDbConnection);
            };
        }  // namespace jni
    }  // namespace odbc
}  // namespace ignite

#endif // _IGNITE_ODBC_JNI_DOCUMENTDB_CONNECTION
