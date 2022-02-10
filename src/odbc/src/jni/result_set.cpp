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

using ignite::odbc::common::concurrent::SharedPointer;
using ignite::odbc::jni::java::GlobalJObject;
using ignite::odbc::jni::java::JniContext;
using ignite::odbc::jni::java::JniErrorInfo;

namespace ignite 
{
    namespace odbc 
    {
        namespace jni 
        {
            ResultSet::ResultSet(SharedPointer< JniContext > jniContext,
                                SharedPointer< GlobalJObject > resultSet)
            : _jniContext(jniContext), _resultSet(resultSet) {
            }

            ResultSet::~ResultSet() {
                if (_jniContext.Get() != nullptr) {
                    if (_resultSet.Get() != nullptr) {
                        JniErrorInfo errInfo;
                        _jniContext.Get()->ResultSetClose(_resultSet, errInfo);
                        _resultSet = nullptr;
                    }
                    _jniContext = nullptr;
                }
            }

            bool Next(bool& hasNext, JniErrorInfo& errInfo) {
                return false;
            }

            bool GetString(const int columnIndex, std::string& value,
                           bool& wasNull, JniErrorInfo& errInfo) {
                return false;
            }

            bool GetString(const std::string& columnName, std::string& value,
                           bool& wasNull, JniErrorInfo& errInfo) {
                return false;
            }

            bool GetInt(const int columnIndex, int& value, bool& wasNull,
                        JniErrorInfo& errInfo) {
                return false;
            }

            bool GetInt(const std::string& columnName, int& value,
                        bool& wasNull, JniErrorInfo& errInfo) {
                return false;
            }

        }  // namespace jni
    }  // namespace odbc
}  // namespace ignite