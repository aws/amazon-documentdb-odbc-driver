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

namespace ignite {
    namespace odbc {
        namespace jni {
            ResultSet::ResultSet(SharedPointer< JniContext > jniContext,
                                SharedPointer< GlobalJObject > resultSet)
                : _jniContext(jniContext), _resultSet(resultSet) {
            }

            ResultSet::~ResultSet() {
                JniErrorInfo errInfo;
                Close(errInfo);
                if (_jniContext.IsValid()) {
                    _jniContext = nullptr;
                }
            }

            bool ResultSet::Close(JniErrorInfo& errInfo) {
                if (_jniContext.IsValid() && _resultSet.IsValid()) {
                    bool success =
                        _jniContext.Get()->ResultSetClose(_resultSet, errInfo);
                    _resultSet = nullptr;
                    return success;
                }
                return true;
            }
            
            bool ResultSet::Next(bool& hasNext, JniErrorInfo& errInfo) {
                return _jniContext.Get()->ResultSetNext(_resultSet, hasNext, errInfo);
            }

            bool ResultSet::GetString(const int columnIndex, std::string& value,
                                      bool& wasNull, JniErrorInfo& errInfo) {
                return _jniContext.Get()->ResultSetGetString(
                    _resultSet, columnIndex, value, wasNull, errInfo);
            }

            bool ResultSet::GetString(const std::string& columnName,
                                      std::string& value,
                           bool& wasNull, JniErrorInfo& errInfo) {
                return _jniContext.Get()->ResultSetGetString(
                    _resultSet, columnName, value, wasNull, errInfo);
            }

            bool ResultSet::GetInt(const int columnIndex, int& value,
                                   bool& wasNull,
                        JniErrorInfo& errInfo) {
                return _jniContext.Get()->ResultSetGetInt(
                    _resultSet, columnIndex, value, wasNull, errInfo);
            }

            bool ResultSet::GetInt(const std::string& columnName, int& value,
                        bool& wasNull, JniErrorInfo& errInfo) {
                return _jniContext.Get()->ResultSetGetInt(
                    _resultSet, columnName, value, wasNull, errInfo);
            }
        }  // namespace jni
    }  // namespace odbc
}  // namespace ignite