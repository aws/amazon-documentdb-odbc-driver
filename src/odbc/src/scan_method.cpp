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

#include <ignite/common/utils.h>

#include "ignite/odbc/scan_method.h"

namespace ignite
{
    namespace odbc
    {
        ScanMethod::Type ScanMethod::FromString(const std::string& val, Type dflt)
        {
            std::string lowerVal = common::ToLower(val);

            common::StripSurroundingWhitespaces(lowerVal);

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

        std::string ScanMethod::ToString(Type val)
        {
            switch (val)
            {
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
    }
}
