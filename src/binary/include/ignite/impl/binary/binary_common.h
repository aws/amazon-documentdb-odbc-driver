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

#ifndef _IGNITE_IMPL_BINARY_BINARY_COMMON
#define _IGNITE_IMPL_BINARY_BINARY_COMMON

#include <stdint.h>

namespace ignite
{
    namespace impl
    {
        namespace binary
        {
            /** Header: null. */
            const int8_t IGNITE_HDR_NULL = 101;

            /** Header: handle. */
            const int8_t IGNITE_HDR_HND = 102;

            /** Header: full form. */
            const int8_t IGNITE_HDR_FULL = 103;

            /** Binary protocol version.  */
            const int8_t IGNITE_PROTO_VER = 1;

            /** Protocol version position. */
            const int32_t PROTO_VER_POS = 1;

            /** Header offset: Flags. */
            const int32_t IGNITE_OFFSET_FLAGS = 2;

            /** Header offset: Type ID. */
            const int32_t IGNITE_OFFSET_TYPE_ID = 4;

            /** Header offset: Hash Code. */
            const int32_t IGNITE_OFFSET_HASH_CODE = 8;

            /** Header offset: Object Length. */
            const int32_t IGNITE_OFFSET_LEN = 12;

            /** Header offset: Schema ID. */
            const int32_t IGNITE_OFFSET_SCHEMA_ID = 16;

            /** Header offset: Schema or Raw Offset. */
            const int32_t IGNITE_OFFSET_SCHEMA_OR_RAW_OFF = 20;

            /** Full header length. */
            const int32_t IGNITE_DFLT_HDR_LEN = 24;

            /** Binary header length. */
            const int32_t IGNITE_BINARY_HDR_LEN = 5;

            /** Common header length. */
            const int32_t IGNITE_COMMON_HDR_LEN = 1;

            /** Type: object. */
            const int8_t IGNITE_TYPE_OBJECT = IGNITE_HDR_FULL;

            /** Type: optimized marshaller. */
            const int8_t IGNITE_TYPE_OPTM_MARSH = -2;

            // JDBC types. 

            /** Type: bit. */
            const int16_t JDBC_TYPE_BIT = -7;

            /** Type: tinyint. */
            const int16_t JDBC_TYPE_TINYINT = -6;

            /** Type: snallint. */
            const int16_t JDBC_TYPE_SMALLINT = 5;

            /** Type: integer. */
            const int16_t JDBC_TYPE_INTEGER = 4;

            /** Type: bigint. */
            const int16_t JDBC_TYPE_BIGINT = -5;

            /** Type: float. */
            const int16_t JDBC_TYPE_FLOAT = 6;

            /** Type: real. */
            const int16_t JDBC_TYPE_REAL = 7;

            /** Type: double. */
            const int16_t JDBC_TYPE_DOUBLE = 8;

            /** Type: numeric. */
            const int16_t JDBC_TYPE_NUMERIC = 2;

            /** Type: decimal. */
            const int16_t JDBC_TYPE_DECIMAL = 3;

            /** Type: char. */
            const int16_t JDBC_TYPE_CHAR = 1;

            /** Type: varchar. */
            const int16_t JDBC_TYPE_VARCHAR = 12;

            /** Type: longvarchar. */
            const int16_t JDBC_TYPE_LONGVARCHAR = -1;

            /** Type: date. */
            const int16_t JDBC_TYPE_DATE = 91;

            /** Type: time. */
            const int16_t JDBC_TYPE_TIME = 92;

            /** Type: timestamp. */
            const int16_t JDBC_TYPE_TIMESTAMP = 93;

            /** Type: binary. */
            const int16_t JDBC_TYPE_BINARY = -2;

            /** Type: varbinary. */
            const int16_t JDBC_TYPE_VARBINARY = -3;

            /** Type: long varbinary. */
            const int16_t JDBC_TYPE_LONGVARBINARY = -4;

            /** Type: blob. */
            const int16_t JDBC_TYPE_BLOB = 2004;

            /** Type: clob. */
            const int16_t JDBC_TYPE_CLOB = 2005;

            /** Type: boolean. */
            const int16_t JDBC_TYPE_BOOLEAN = 16;

            /** Type: array. */
            const int16_t JDBC_TYPE_ARRAY = 2003;

            /** Type: struct. */
            const int16_t JDBC_TYPE_STRUCT = 2002;

            /** Type: java object. */
            const int16_t JDBC_TYPE_JAVA_OBJECT = 2000;

            /** Type: rowid. */
            const int16_t JDBC_TYPE_ROWID = -8;

            /** Type: nchar. */
            const int16_t JDBC_TYPE_NCHAR = -15;

            /** Type: nvarchar. */
            const int16_t JDBC_TYPE_NVARCHAR = -9;

            /** Type: longnvarchar. */
            const int16_t JDBC_TYPE_LONGNVARCHAR = -16;

            /** Type: nclob. */
            const int16_t JDBC_TYPE_NCLOB = 2011;

            /** Type: sqlxml. */
            const int16_t JDBC_TYPE_SQLXML = 2009;

            /** Type: ref cursor. */
            const int16_t JDBC_TYPE_REF_CURSOR = 2012;

            /** Type: null. */
            const int16_t JDBC_TYPE_NULL = 0;

            // Ignite enum types

            /** Type: unsigned byte. */
            const int8_t IGNITE_TYPE_BYTE = 1;

            /** Type: short. */
            const int8_t IGNITE_TYPE_SHORT = 2;

            /** Type: int. */
            const int8_t IGNITE_TYPE_INT = 3;

            /** Type: long. */
            const int8_t IGNITE_TYPE_LONG = 4;

            /** Type: float. */
            const int8_t IGNITE_TYPE_FLOAT = 5;

            /** Type: double. */
            const int8_t IGNITE_TYPE_DOUBLE = 6;

            /** Type: char. */
            const int8_t IGNITE_TYPE_CHAR = 7;

            /** Type: boolean. */
            const int8_t IGNITE_TYPE_BOOL = 8;

            /** Type: string. */
            const int8_t IGNITE_TYPE_STRING = 9;

            /** Type: UUID. */
            const int8_t IGNITE_TYPE_UUID = 10;

            /** Type: date. */
            const int8_t IGNITE_TYPE_DATE = 11;

            /** Type: unsigned byte array. */
            const int8_t IGNITE_TYPE_ARRAY_BYTE = 12;

            /** Type: short array. */
            const int8_t IGNITE_TYPE_ARRAY_SHORT = 13;

            /** Type: int array. */
            const int8_t IGNITE_TYPE_ARRAY_INT = 14;

            /** Type: long array. */
            const int8_t IGNITE_TYPE_ARRAY_LONG = 15;

            /** Type: float array. */
            const int8_t IGNITE_TYPE_ARRAY_FLOAT = 16;

            /** Type: double array. */
            const int8_t IGNITE_TYPE_ARRAY_DOUBLE = 17;

            /** Type: char array. */
            const int8_t IGNITE_TYPE_ARRAY_CHAR = 18;

            /** Type: boolean array. */
            const int8_t IGNITE_TYPE_ARRAY_BOOL = 19;

            /** Type: string array. */
            const int8_t IGNITE_TYPE_ARRAY_STRING = 20;

            /** Type: UUID array. */
            const int8_t IGNITE_TYPE_ARRAY_UUID = 21;

            /** Type: date array. */
            const int8_t IGNITE_TYPE_ARRAY_DATE = 22;

            /** Type: object array. */
            const int8_t IGNITE_TYPE_ARRAY = 23;

            /** Type: collection. */
            const int8_t IGNITE_TYPE_COLLECTION = 24;

            /** Type: map. */
            const int8_t IGNITE_TYPE_MAP = 25;

            /** Type: map entry. */
            const int8_t IGNITE_TYPE_MAP_ENTRY = 26;

            /** Type: binary object. */
            const int8_t IGNITE_TYPE_BINARY = 27;

            /** Type: enum. */
            const int8_t IGNITE_TYPE_ENUM = 28;

            /** Type: enum array. */
            const int8_t IGNITE_TYPE_ARRAY_ENUM = 29;

            /** Type: decimal. */
            const int8_t IGNITE_TYPE_DECIMAL = 30;

            /** Type: decimal array. */
            const int8_t IGNITE_TYPE_ARRAY_DECIMAL = 31;

            /** Type: class. */
            const int8_t IGNITE_TYPE_CLASS = 32;

            /** Type: timestamp. */
            const int8_t IGNITE_TYPE_TIMESTAMP = 33;

            /** Type: timestamp array. */
            const int8_t IGNITE_TYPE_ARRAY_TIMESTAMP = 34;

            /** Type: proxy. */
            const int8_t IGNITE_TYPE_PROXY = 35;

            /** Type: time. */
            const int8_t IGNITE_TYPE_TIME = 36;

            /** Type: time array. */
            const int8_t IGNITE_TYPE_ARRAY_TIME = 37;

            /** Type: binary enum. */
            const int8_t IGNITE_TYPE_BINARY_ENUM = 38;

            /** Read/write single object. */
            const int32_t IGNITE_BINARY_MODE_SINGLE = 0;

            /** Read/write array. */
            const int32_t IGNITE_BINARY_MODE_ARRAY = 1;

            /** Read/write collection. */
            const int32_t IGNITE_BINARY_MODE_COL = 2;

            /** Read/write map. */
            const int32_t IGNITE_BINARY_MODE_MAP = 3;

            /** User type flag. */
            const int16_t IGNITE_BINARY_FLAG_USER_TYPE = 0x0001;

            /** Flag: schema exists. */
            const int16_t IGNITE_BINARY_FLAG_HAS_SCHEMA = 0x0002;

            /** Flag indicating that object has raw data. */
            const int16_t IGNITE_BINARY_FLAG_HAS_RAW = 0x0004;

            /** Flag indicating that schema field offset is one byte long. */
            const int16_t IGNITE_BINARY_FLAG_OFFSET_ONE_BYTE = 0x0008;

            /** Flag indicating that schema field offset is two byte long. */
            const int16_t IGNITE_BINARY_FLAG_OFFSET_TWO_BYTES = 0x0010;

            /** Flag: compact footer, no field IDs. */
            const int16_t IGNITE_BINARY_FLAG_COMPACT_FOOTER = 0x0020;
        }
    }
}

#endif //_IGNITE_IMPL_BINARY_BINARY_COMMON
