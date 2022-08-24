/*
 * Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */

#ifndef _DOCUMENTDB_ODBC_LOG_LEVEL
#define _DOCUMENTDB_ODBC_LOG_LEVEL

#include <string>

namespace documentdb {
namespace odbc {
/** Log Level enum. */
struct LogLevel {
  enum class Type { DEBUG_LEVEL = 0, INFO_LEVEL, ERROR_LEVEL, OFF, UNKNOWN };

  /**
   * Convert log level from string.
   *
   * @param val String value.
   * @param dflt Default value to return on error.
   * @return Corresponding enum value.
   */
  static Type FromString(const std::string& val, Type dflt = Type::UNKNOWN);

  /**
   * Convert log level to string.
   *
   * @param val Value to convert.
   * @return String value.
   */
  static std::string ToString(Type val);

  /**
   * Convert log level to descriptive text.
   *
   * @param val Value to convert.
   * @return wide String value.
   */
  static std::wstring ToText(Type val);

  inline static int ToInt(Type val) {
    return static_cast< int >(val);
  }

  inline static Type FromInt(int val) {
    return static_cast< Type >(val);
  }
};
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_LOG_LEVEL
