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

#include "ignite/odbc/scan_method.h"

#include <ignite/common/utils.h>

namespace ignite {
namespace odbc {
ScanMethod::Type ScanMethod::FromString(const std::string& val, Type dflt) {
  std::string lowerVal = common::ToLower(val);

  common::StripSurroundingWhitespaces(lowerVal);

  common::SpaceToUnderscore(lowerVal);

  if (lowerVal == "random")
    return ScanMethod::Type::RANDOM;

  if (lowerVal == "id_forward")
    return ScanMethod::Type::ID_FORWARD;

  if (lowerVal == "id_reverse")
    return ScanMethod::Type::ID_REVERSE;

  if (lowerVal == "all")
    return ScanMethod::Type::ALL;

  return dflt;
}

std::string ScanMethod::ToString(Type val) {
  switch (val) {
    case ScanMethod::Type::ID_FORWARD:
      return "id_forward";

    case ScanMethod::Type::ID_REVERSE:
      return "id_reverse";

    case ScanMethod::Type::ALL:
      return "all";

    case ScanMethod::Type::RANDOM:
      return "random";

    default:
      return "unknown";
  }
}

std::string ScanMethod::ToJdbcString(Type val) {
  switch (val) {
    case ScanMethod::Type::ID_FORWARD:
      return "idForward";

    case ScanMethod::Type::ID_REVERSE:
      return "idReverse";

    case ScanMethod::Type::ALL:
      return "all";

    default:
      return "random";
  }
}
}  // namespace odbc
}  // namespace ignite
