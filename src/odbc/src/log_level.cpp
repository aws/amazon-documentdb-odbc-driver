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

#include "ignite/odbc/log_level.h"

#include <ignite/common/utils.h>

namespace ignite {
namespace odbc {
LogLevel::Type LogLevel::FromString(const std::string& val,
                                                Type dflt) {
  std::string lowerVal = common::ToLower(val);

  common::StripSurroundingWhitespaces(lowerVal);

  if (lowerVal == "debug")
    return LogLevel::Type::DEBUG_LEVEL;

  if (lowerVal == "info")
    return LogLevel::Type::INFO_LEVEL;

  if (lowerVal == "error")
    return LogLevel::Type::ERROR_LEVEL;

  if (lowerVal == "off")
    return LogLevel::Type::OFF;

  return dflt;
}

std::string LogLevel::ToString(Type val) {
  switch (val) {
    case LogLevel::Type::DEBUG_LEVEL:
      return "debug";

    case LogLevel::Type::INFO_LEVEL:
      return "info";

    case LogLevel::Type::ERROR_LEVEL:
      return "error";

    case LogLevel::Type::OFF:
      return "off";

    default:
      return "unknown";
  }
}
}  // namespace odbc
}  // namespace ignite
