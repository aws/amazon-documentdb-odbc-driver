/*
 * Copyright <2021> Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// clang-format off
#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#include "pch.h"
#include "unit_test_helper.h"
#include "it_odbc_helper.h"
#include "../timestreamsqlodbc/version.h"
#include <codecvt>
#ifndef WIN32
#include <climits>
#endif
// clang-format on

class TestSQLGetInfo : public testing::Test {
   public:
    TestSQLGetInfo() {
    }

    void SetUp() {
        if (std::getenv("NOT_CONNECTED")) {
            GTEST_SKIP();
        }
        ITDriverConnect((SQLTCHAR*)(conn_string().c_str()), &m_env, &m_conn, true,
                        true);
    }

    void TearDown() {
        if (std::getenv("NOT_CONNECTED")) {
            GTEST_SKIP();
        }
        SQLDisconnect(m_conn);
        SQLFreeHandle(SQL_HANDLE_ENV, m_env);
    }

    ~TestSQLGetInfo() {
        // cleanup any pending stuff, but no exceptions allowed
    }

    SQLHENV m_env = SQL_NULL_HENV;
    SQLHDBC m_conn = SQL_NULL_HDBC;
};

// 1 for ver1 >= ver2, 0 for ver1 < ver2, -1 for error
int Ver1GEVer2(test_string ver_1_str, test_string ver_2_str) {
    auto VersionSplit = [&](std::vector< unsigned long >& output,
                            test_string& input, test_string delim) {
        try {
            std::string delim_str = tchar_to_string((const SQLTCHAR*)delim.c_str());
            std::string input_str = tchar_to_string((const SQLTCHAR*)input.c_str());
            size_t start = 0;
            size_t end = input_str.find(delim_str);
            while (end != std::string::npos) {
                output.push_back(std::stoul(input_str.substr(start, end - start)));
                start = end + delim_str.length();
                end = input_str.find(delim_str, start);
            }
            output.push_back(std::stoul(input_str.substr(start, end)));
        } catch (...) {
            output.clear();
        }
    };

    std::vector< unsigned long > ver_1;
    std::vector< unsigned long > ver_2;
    VersionSplit(ver_1, ver_1_str,CREATE_STRING("."));
    VersionSplit(ver_2, ver_2_str, CREATE_STRING("."));
    if ((ver_1.size() == 0) || (ver_2.size() == 0))
        return -1;

    size_t cnt = ((ver_1.size() < ver_2.size()) ? ver_1.size() : ver_2.size());
    for (size_t i = 0; i < cnt; i++) {
        if (ver_1[i] != ver_2[i])
            return (ver_1[i] >= ver_2[i]) ? 1 : 0;
    }
    if (ver_1.size() != ver_2.size())
        return (ver_1.size() > ver_2.size()) ? 1 : 0;

    // They are identical
    return 1;
}

// Test template for SQLGetInfo
#define TEST_SQL_GET_INFO_STRING(test_name, info_type, expected_value)         \
    TEST_F(TestSQLGetInfo, test_name) {                                        \
        if (std::getenv("NOT_CONNECTED")) {                                    \
            GTEST_SKIP();                                                      \
        }                                                                      \
        SQLTCHAR info_value_ptr[1024];                                         \
        SQLSMALLINT string_length_ptr;                                         \
        SQLRETURN ret =                                                        \
            SQLGetInfo(m_conn, info_type, (SQLTCHAR*)info_value_ptr,       \
                       IT_SIZEOF(info_value_ptr), &string_length_ptr);                              \
        LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);                        \
        EXPECT_EQ(to_test_string(tchar_to_string(info_value_ptr)), test_string(expected_value));                    \
    }

// Test template for SQLGetInfo
#define TEST_SQL_GET_INFO_VERSION_GE(test_name, info_type, expected_value) \
    TEST_F(TestSQLGetInfo, test_name) {                                    \
        SQLTCHAR info_value_ptr[1024];                                     \
        SQLSMALLINT string_length_ptr;                                     \
        SQLRETURN ret =                                                    \
            SQLGetInfo(m_conn, info_type, info_value_ptr,                  \
                       IT_SIZEOF(info_value_ptr), &string_length_ptr);     \
        LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);                    \
        test_string info_value_str =                                       \
            to_test_string(tchar_to_string(info_value_ptr));               \
        EXPECT_EQ(Ver1GEVer2(info_value_str, expected_value), 1);          \
    }

// Test template for SQLGetInfo
#define TEST_SQL_GET_INFO_UINT_MASK(test_name, info_type, expected_value) \
    TEST_F(TestSQLGetInfo, test_name) {                                   \
        SQLUINTEGER info_value_ptr;                                       \
        SQLSMALLINT string_length_ptr;                                    \
        SQLRETURN ret =                                                   \
            SQLGetInfo(m_conn, info_type, &info_value_ptr,                \
                       sizeof(info_value_ptr), &string_length_ptr);       \
        LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);                   \
        EXPECT_EQ((size_t)info_value_ptr, (size_t)expected_value);        \
    }

// Test template for SQLGetInfo
#define TEST_SQL_GET_INFO_UINT16(test_name, info_type, expected_value) \
    TEST_F(TestSQLGetInfo, test_name) {                                \
        SQLUSMALLINT info_value_ptr;                                   \
        SQLSMALLINT string_length_ptr;                                 \
        SQLRETURN ret =                                                \
            SQLGetInfo(m_conn, info_type, &info_value_ptr,             \
                       sizeof(info_value_ptr), &string_length_ptr);    \
        LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);                \
        EXPECT_EQ(info_value_ptr, expected_value);                     \
    }

/////////////////
// Driver Info //
/////////////////

#if defined(WIN32)
TEST_SQL_GET_INFO_STRING(SQLDriverName, SQL_DRIVER_NAME, CREATE_STRING("timestreamsqlodbc.dll"))
#elif defined(__APPLE__)
TEST_SQL_GET_INFO_STRING(SQLDriverName, SQL_DRIVER_NAME, CREATE_STRING("libtimestreamsqlodbc.dylib"))
#endif
TEST_SQL_GET_INFO_STRING(SQLDriverODBCVer, SQL_DRIVER_ODBC_VER, CREATE_STRING("03.51"))

test_string version = CREATE_STRING("") TIMESTREAMDRIVERVERSION;
TEST_SQL_GET_INFO_STRING(SQLDriverVer, SQL_DRIVER_VER, version)

TEST_SQL_GET_INFO_UINT_MASK(SQLGetDataExtensions, SQL_GETDATA_EXTENSIONS,
                         (SQL_GD_ANY_COLUMN | SQL_GD_ANY_ORDER | SQL_GD_BOUND
                          | SQL_GD_BLOCK))
TEST_SQL_GET_INFO_STRING(SQLSearchPatternEscape, SQL_SEARCH_PATTERN_ESCAPE,
                         CREATE_STRING(""))

//////////////////////
// Data Source Info //
//////////////////////

TEST_SQL_GET_INFO_UINT16(SQLCursorCommitBehavior, SQL_CURSOR_COMMIT_BEHAVIOR,
                         SQL_CB_CLOSE)
TEST_SQL_GET_INFO_UINT16(SQLTxnCapable, SQL_TXN_CAPABLE, SQL_TC_NONE)
TEST_SQL_GET_INFO_UINT16(SQLConcatNullBehavior, SQL_CONCAT_NULL_BEHAVIOR,
                         SQL_CB_NULL)
TEST_SQL_GET_INFO_STRING(SQLSchemaTerm, SQL_SCHEMA_TERM, CREATE_STRING("schema"))
TEST_SQL_GET_INFO_STRING(SQLCatalogTerm, SQL_CATALOG_TERM, CREATE_STRING("database"))

///////////////
// DBMS Info //
///////////////

TEST_SQL_GET_INFO_STRING(SQLDBMSName, SQL_DBMS_NAME, CREATE_STRING("Amazon Timestream"))
TEST_SQL_GET_INFO_VERSION_GE(SQLDBMSVer, SQL_DBMS_VER, CREATE_STRING("0.2.0"))

///////////////////
// Supported SQL //
///////////////////

TEST_SQL_GET_INFO_STRING(SQLColumnAlias, SQL_COLUMN_ALIAS, CREATE_STRING("Y"))
TEST_SQL_GET_INFO_UINT16(SQLGroupBy, SQL_GROUP_BY,
                         SQL_GB_GROUP_BY_CONTAINS_SELECT)
TEST_SQL_GET_INFO_STRING(SQLIdentifierQuoteChar, SQL_IDENTIFIER_QUOTE_CHAR,
                         CREATE_STRING("\""))
TEST_SQL_GET_INFO_UINT_MASK(SQLOJCapabilities, SQL_OJ_CAPABILITIES,
                            SQL_OJ_LEFT | SQL_OJ_RIGHT | SQL_OJ_NOT_ORDERED
                                | SQL_OJ_ALL_COMPARISON_OPS)
TEST_SQL_GET_INFO_UINT_MASK(SQLSchemaUsage, SQL_SCHEMA_USAGE, 0)
TEST_SQL_GET_INFO_UINT16(SQLQuotedIdentifierCase, SQL_QUOTED_IDENTIFIER_CASE,
                         SQL_IC_SENSITIVE)
TEST_SQL_GET_INFO_STRING(SQLSpecialCharacters, SQL_SPECIAL_CHARACTERS, CREATE_STRING("_"))
TEST_SQL_GET_INFO_UINT_MASK(SQLODBCInterfaceConformance,
                            SQL_ODBC_INTERFACE_CONFORMANCE, SQL_OIC_CORE)
TEST_SQL_GET_INFO_UINT_MASK(SQLSQLConformance, SQL_SQL_CONFORMANCE,
                            SQL_SC_SQL92_ENTRY)
TEST_SQL_GET_INFO_UINT_MASK(SQLCatalogUsage, SQL_CATALOG_USAGE, SQL_CU_DML_STATEMENTS)
TEST_SQL_GET_INFO_UINT16(SQLCatalogLocation, SQL_CATALOG_LOCATION, SQL_CL_START)
TEST_SQL_GET_INFO_STRING(SQLCatalogNameSeparator, SQL_CATALOG_NAME_SEPARATOR,
                         CREATE_STRING("."))
TEST_SQL_GET_INFO_UINT_MASK(SQLSQL92Predicates, SQL_SQL92_PREDICATES,
                            SQL_SP_BETWEEN | SQL_SP_COMPARISON | SQL_SP_IN
                                | SQL_SP_ISNULL | SQL_SP_LIKE)
TEST_SQL_GET_INFO_UINT_MASK(SQLSQL92RelationalJoinOperators,
                            SQL_SQL92_RELATIONAL_JOIN_OPERATORS,
                            SQL_SRJO_CROSS_JOIN | SQL_SRJO_INNER_JOIN
                                | SQL_SRJO_LEFT_OUTER_JOIN
                                | SQL_SRJO_RIGHT_OUTER_JOIN)
TEST_SQL_GET_INFO_UINT_MASK(SQLSQL92ValueExpressions,
                            SQL_SQL92_VALUE_EXPRESSIONS,
                            SQL_SVE_CASE | SQL_SVE_CAST)
TEST_SQL_GET_INFO_UINT_MASK(SQLDatetimeLiterals, SQL_DATETIME_LITERALS, 0)
TEST_SQL_GET_INFO_STRING(SQLOrderByColumnsInSelect,
                         SQL_ORDER_BY_COLUMNS_IN_SELECT, CREATE_STRING("Y"))
TEST_SQL_GET_INFO_STRING(SQLCatalogName, SQL_CATALOG_NAME, CREATE_STRING("Y"))

////////////////
// Conversion //
////////////////

TEST_SQL_GET_INFO_UINT_MASK(SQLConvertInteger, SQL_CONVERT_INTEGER, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertSmallint, SQL_CONVERT_SMALLINT, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertTinyint, SQL_CONVERT_TINYINT, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertBit, SQL_CONVERT_BIT, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertVarchar, SQL_CONVERT_VARCHAR, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertBigint, SQL_CONVERT_BIGINT, SQL_CVT_DOUBLE)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertDecimal, SQL_CONVERT_DECIMAL, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertDouble, SQL_CONVERT_DOUBLE, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertFloat, SQL_CONVERT_FLOAT, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertNumeric, SQL_CONVERT_NUMERIC, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertReal, SQL_CONVERT_REAL, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertDate, SQL_CONVERT_DATE, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertTime, SQL_CONVERT_TIME, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertTimestamp, SQL_CONVERT_TIMESTAMP, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertBinary, SQL_CONVERT_BINARY, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertLongvarbinary, SQL_CONVERT_LONGVARBINARY,
                            0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertVarbinary, SQL_CONVERT_VARBINARY, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertChar, SQL_CONVERT_CHAR, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertLongVarchar, SQL_CONVERT_LONGVARCHAR, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertWChar, SQL_CONVERT_WCHAR, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertWLongVarchar, SQL_CONVERT_WLONGVARCHAR,
                            0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertWVarchar, SQL_CONVERT_WVARCHAR, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLConvertGuid, SQL_CONVERT_GUID, 0)

//////////////////////
// Scalar Functions //
//////////////////////

TEST_SQL_GET_INFO_UINT_MASK(SQLConvertFunctions, SQL_CONVERT_FUNCTIONS,
                            SQL_FN_CVT_CAST)
TEST_SQL_GET_INFO_UINT_MASK(SQLNumericFunctions, SQL_NUMERIC_FUNCTIONS,
                            SQL_FN_NUM_ABS | SQL_FN_NUM_ATAN | SQL_FN_NUM_ATAN2
                                | SQL_FN_NUM_COS | SQL_FN_NUM_COT
                                | SQL_FN_NUM_DEGREES | SQL_FN_NUM_FLOOR
                                | SQL_FN_NUM_LOG | SQL_FN_NUM_LOG10
                                | SQL_FN_NUM_PI | SQL_FN_NUM_POWER
                                | SQL_FN_NUM_RADIANS | SQL_FN_NUM_ROUND
                                | SQL_FN_NUM_SIGN | SQL_FN_NUM_SIN
                                | SQL_FN_NUM_SQRT | SQL_FN_NUM_TAN)
TEST_SQL_GET_INFO_UINT_MASK(SQLStringFunctions, SQL_STRING_FUNCTIONS,
                            SQL_FN_STR_ASCII | SQL_FN_STR_LENGTH
                                | SQL_FN_STR_LTRIM | SQL_FN_STR_REPLACE
                                | SQL_FN_STR_RTRIM | SQL_FN_STR_SUBSTRING)
TEST_SQL_GET_INFO_UINT_MASK(SQLSystemFunctions, SQL_SYSTEM_FUNCTIONS,
                            SQL_FN_SYS_IFNULL)
TEST_SQL_GET_INFO_UINT_MASK(SQLTimedateAddIntervals, SQL_TIMEDATE_ADD_INTERVALS,
                            0)
TEST_SQL_GET_INFO_UINT_MASK(SQLTimedateDiffIntervals,
                            SQL_TIMEDATE_DIFF_INTERVALS, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLTimedateFunctions, SQL_TIMEDATE_FUNCTIONS,
                            SQL_FN_TD_CURDATE | SQL_FN_TD_DAYOFMONTH
                                | SQL_FN_TD_MONTH | SQL_FN_TD_MONTHNAME
                                | SQL_FN_TD_NOW | SQL_FN_TD_YEAR)
TEST_SQL_GET_INFO_UINT_MASK(SQLSQL92DatetimeFunctions,
                            SQL_SQL92_DATETIME_FUNCTIONS, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLSQL92NumericValueFunctions,
                            SQL_SQL92_NUMERIC_VALUE_FUNCTIONS, 0)
TEST_SQL_GET_INFO_UINT_MASK(SQLSQL92StringFunctions, SQL_SQL92_STRING_FUNCTIONS,
                            SQL_SSF_LOWER | SQL_SSF_UPPER)

////////////
// Limits //
////////////

TEST_SQL_GET_INFO_UINT16(SQLMaxIdentifierLen, SQL_MAX_IDENTIFIER_LEN, SHRT_MAX)
TEST_SQL_GET_INFO_UINT16(SQLMaxColumnsInGroupBy, SQL_MAX_COLUMNS_IN_GROUP_BY,
                         0)
TEST_SQL_GET_INFO_UINT16(SQLMaxColumnsInOrderBy, SQL_MAX_COLUMNS_IN_ORDER_BY,
                         0)
TEST_SQL_GET_INFO_UINT16(SQLMaxColumnsInSelect, SQL_MAX_COLUMNS_IN_SELECT, 0)
