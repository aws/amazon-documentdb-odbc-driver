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

#ifndef _IGNITE_ODBC_JNI_RESULT_SET
#define _IGNITE_ODBC_JNI_RESULT_SET

#include <ignite/odbc/common/concurrent.h>
#include <ignite/odbc/jni/java.h>

#include <string>

using ignite::odbc::common::concurrent::SharedPointer;
using ignite::odbc::jni::java::GlobalJObject;
using ignite::odbc::jni::java::JniContext;
using ignite::odbc::jni::java::JniErrorCode;
using ignite::odbc::jni::java::JniErrorInfo;

namespace ignite {
namespace odbc {
namespace jni {
/**
 * A wrapper class for the ResultSet java class.
 */
class ResultSet {
  friend class DatabaseMetaData;

 public:
  /**
   * Destructs the current object.
   */
  ~ResultSet();

  /**
   * Closes the current ResultSet object.
   *
   * @return a JniErrorCode indicating success or failure.
   */
  JniErrorCode Close(JniErrorInfo& errInfo);

  /**
   * Gets an indicator of whether the ResultSet is open.
   *
   * @return true if open, false, otherwise.
   */
  bool IsOpen() {
    return _resultSet.IsValid();
  }

  /**
   * Attempts to position the cursor to the next record in the result set.
   * If the cursor moves, the hasNext variable will be true, false, otherwise.
   *
   * @return a JniErrorCode indicating success or failure.
   */
  JniErrorCode Next(bool& hasNext, JniErrorInfo& errInfo);

  /**
   * Gets a value on the current row of the result set for the
   * given columnIndex (1-indexed). If a value exists, the value
   * is set. If the value is null, the wasNull will be set to
   * true, false, otherwise.
   *
   * @return a JniErrorCode indicating success or failure.
   */
  JniErrorCode GetString(const int columnIndex,
                         boost::optional< std::string >& value,
                         JniErrorInfo& errInfo);

  /**
   * Gets a value on the current row of the result set for the
   * given columnName. If a value exists, the value
   * is set. If the value is null, the wasNull will be set to
   * true, false, otherwise.
   *
   * @return a JniErrorCode indicating success or failure.
   */
  JniErrorCode GetString(const std::string& columnName,
                         boost::optional< std::string >& value,
                         JniErrorInfo& errInfo);

  /**
   * Gets a value on the current row of the result set for the
   * given columnIndex (1-indexed). If a value exists, the value
   * is set. If the value is null, the wasNull will be set to
   * true, false, otherwise.
   *
   * @return a JniErrorCode indicating success or failure.
   */
  JniErrorCode GetInt(const int columnIndex, boost::optional< int >& value,
                      JniErrorInfo& errInfo);

  /**
   * Gets a value on the current row of the result set for the
   * given columnName. If a value exists, the value
   * is set. If the value is null, the wasNull will be set to
   * true, false, otherwise.
   *
   * @return a JniErrorCode indicating success or failure.
   */
  JniErrorCode GetInt(const std::string& columnName,
                      boost::optional< int >& value, JniErrorInfo& errInfo);

  /**
   * Gets a value on the current row of the result set for the
   * given columnIndex (1-indexed). If a value exists, the value
   * is set. If the value is null, the wasNull will be set to
   * true, false, otherwise.
   *
   * @return a JniErrorCode indicating success or failure.
   */
  JniErrorCode GetSmallInt(const int columnIndex,
                           boost::optional< int16_t >& value,
                           JniErrorInfo& errInfo);

  /**
   * Gets a value on the current row of the result set for the
   * given columnName. If a value exists, the value
   * is set. If the value is null, the wasNull will be set to
   * true, false, otherwise.
   *
   * @return a JniErrorCode indicating success or failure.
   */
  JniErrorCode GetSmallInt(const std::string& columnName,
                           boost::optional< int16_t >& value,
                           JniErrorInfo& errInfo);

 private:
  /**
   * Constructs a new instancee of ResultSet.
   */
  ResultSet(SharedPointer< JniContext >& jniContext,
            SharedPointer< GlobalJObject >& resultSet);

  /** The JNI context */
  SharedPointer< JniContext > _jniContext;

  /** The ResultSet Java object */
  SharedPointer< GlobalJObject > _resultSet;
};
}  // namespace jni
}  // namespace odbc
}  // namespace ignite

#endif  // _IGNITE_ODBC_JNI_RESULT_SET
