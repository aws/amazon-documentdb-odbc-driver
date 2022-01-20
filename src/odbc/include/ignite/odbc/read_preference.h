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
         * Convert mode to string. 
         *
	     * @param val Value to convert.
         * @return String value.
         */
        static std::string ToString(Type val);
        
		};
    }
}
