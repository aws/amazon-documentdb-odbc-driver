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

#include <ignite/common/utils.h>

#include "ignite/odbc/read_preference.h"

namespace ignite
{
    namespace odbc
    {
        ReadPreference::Type ReadPreference::FromString(const std::string& val, Type dflt)
        {
            std::string lowerVal = common::ToLower(val);

            common::StripSurroundingWhitespaces(lowerVal);

            common::SpaceToUnderscore(lowerVal);

            if (lowerVal == "primary")
                return ReadPreference::Type::PRIMARY;

            if (lowerVal == "primary_preferred")
                return ReadPreference::Type::PRIMARY_PREFERRED;

            if (lowerVal == "secondary")
                return ReadPreference::Type::SECONDARY;

            if (lowerVal == "secondary_preferred")
                return ReadPreference::Type::SECONDARY_PREFERRED;

            if (lowerVal == "nearest")
                return ReadPreference::Type::NEAREST;

            return dflt;
        }

        std::string ReadPreference::ToString(Type val)
        {
            switch (val)
            {
                case ReadPreference::Type::PRIMARY:
                    return "primary";

                case ReadPreference::Type::PRIMARY_PREFERRED:
                    return "primary_preferred";

                case ReadPreference::Type::SECONDARY:
                    return "secondary";

                case ReadPreference::Type::SECONDARY_PREFERRED:
                    return "secondary_preferred";

                case ReadPreference::Type::NEAREST:
                    return "nearest";

                default:
                    return "unknown";
            }
        }

        std::string ReadPreference::ToJdbcString(Type val)
        {
            switch (val)
            {
            case ReadPreference::Type::PRIMARY_PREFERRED:
                return "primaryPreferred";

            case ReadPreference::Type::SECONDARY:
                return "secondary";

            case ReadPreference::Type::SECONDARY_PREFERRED:
                return "secondaryPreferred";

            case ReadPreference::Type::NEAREST:
                return "nearest";

            default:
                return "primary";
            }
        }
    }
}
