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

#ifndef _IGNITE_ODBC_READ_PREFERENCE
#define _IGNITE_ODBC_READ_PREFERENCE

#include <string>

namespace ignite
{
    namespace odbc
    {
        /** Read Preference enum. */
        struct ReadPreference
        {
            enum class Type
            {
                PRIMARY,
                PRIMARY_PREFERRED,
                SECONDARY,
                SECONDARY_PREFERRED,
                NEAREST,
                UNKNOWN
            };
    
            /**
             * Convert preference from string. 
             *
             * @param val String value.
             * @param dflt Default value to return on error.
             * @return Corresponding enum value.
             */
            static Type FromString(const std::string& val, Type dflt = Type::UNKNOWN);

            /**
             * Convert preference to string. 
             *
             * @param val Value to convert.
             * @return String value.
             */
            static std::string ToString(Type val);

            /**
             * Convert preference to string in JDBC format.
             *
             * @param val Value to convert.
             * @return String value in expected format for JDBC connection string.
             */
            static std::string ToJdbcString(Type val);

        };
    }
}

#endif //_IGNITE_ODBC_READ_PREFERENCE

