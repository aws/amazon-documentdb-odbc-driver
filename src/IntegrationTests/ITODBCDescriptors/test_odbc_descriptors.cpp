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
// clang-format on

const test_string data_set = CREATE_STRING("ODBCTest.IoT");
const test_string single_row = CREATE_STRING("1");
const uint64_t multi_row_cnt = 25;
const uint64_t multi_col_cnt = 7;
const uint64_t single_col_cnt = 1;
const test_string multi_col =
    CREATE_STRING("null, 1, VARCHAR '2', DOUBLE '3.3', true, date '2021-01-01', 1d");

inline void ExecuteQuery(const test_string& column, const test_string& table,
                         const test_string& count, SQLHSTMT* hstmt) {
    test_string statement = QueryBuilder(column, table, count);
    SQLRETURN ret = SQLExecDirect(*hstmt, (SQLTCHAR*)statement.c_str(),
                                  (SQLINTEGER)statement.length());
    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
}

class TestSQLSetDescRec : public Fixture {};

class TestSQLGetDescRec : public Fixture {};

class TestSQLCopyDesc : public Fixture {};

TEST_F(TestSQLCopyDesc, ARD_TO_ARD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_ROW_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_APP_ROW_DESC, &copy, 0, NULL)));
    EXPECT_TRUE(SQL_SUCCEEDED(SQLCopyDesc(ard, copy)));
    SQLSMALLINT left;
    SQLINTEGER left_indicator;
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(copy, 0, SQL_DESC_COUNT, &left, 0, &left_indicator)));
    SQLSMALLINT right;
    SQLINTEGER right_indicator;
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(copy, 0, SQL_DESC_COUNT, &right, 0, &right_indicator)));
    EXPECT_EQ(left, right);
    EXPECT_EQ(left_indicator, right_indicator);
}

TEST_F(TestSQLCopyDesc, ARD_TO_APD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_ROW_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_APP_PARAM_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_GENERAL_ERROR));
}

TEST_F(TestSQLCopyDesc, ARD_TO_IPD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_ROW_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_IMP_PARAM_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_GENERAL_ERROR));
}

TEST_F(TestSQLCopyDesc, ARD_TO_IRD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_ROW_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_IMP_ROW_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_CANNOT_MODIFY_IRD));
}

TEST_F(TestSQLCopyDesc, IRD_TO_IRD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_ROW_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_IMP_ROW_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_CANNOT_MODIFY_IRD));
}

TEST_F(TestSQLCopyDesc, IRD_TO_ARD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_ROW_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_APP_ROW_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_GENERAL_ERROR));
}

TEST_F(TestSQLCopyDesc, IRD_TO_APD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_ROW_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_APP_PARAM_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_GENERAL_ERROR));
}

TEST_F(TestSQLCopyDesc, IRD_TO_IPD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_ROW_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_IMP_PARAM_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_GENERAL_ERROR));
}

TEST_F(TestSQLCopyDesc, APD_TO_APD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_PARAM_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_APP_PARAM_DESC, &copy, 0, NULL)));
    EXPECT_TRUE(SQL_SUCCEEDED(SQLCopyDesc(ard, copy)));
}

TEST_F(TestSQLCopyDesc, APD_TO_ARD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_PARAM_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_APP_ROW_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_GENERAL_ERROR));
}

TEST_F(TestSQLCopyDesc, APD_TO_IPD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_PARAM_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_IMP_PARAM_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_GENERAL_ERROR));
}

TEST_F(TestSQLCopyDesc, APD_TO_IRD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_ROW_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_IMP_ROW_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_CANNOT_MODIFY_IRD));
}

TEST_F(TestSQLCopyDesc, IPD_TO_IPD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_PARAM_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_IMP_PARAM_DESC, &copy, 0, NULL)));
    EXPECT_TRUE(SQL_SUCCEEDED(SQLCopyDesc(ard, copy)));
}

TEST_F(TestSQLCopyDesc, IPD_TO_ARD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_PARAM_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_APP_ROW_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_GENERAL_ERROR));
}

TEST_F(TestSQLCopyDesc, IPD_TO_ARP) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_PARAM_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_APP_ROW_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_GENERAL_ERROR));
}

TEST_F(TestSQLCopyDesc, IPD_TO_IRD) {
    ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt), &m_hstmt);
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_PARAM_DESC, &ard, 0, NULL)));
    SQLHSTMT another_stmt = SQL_NULL_HSTMT;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &another_stmt)));
    SQLHDESC copy = SQL_NULL_HDESC;
    ASSERT_TRUE(SQL_SUCCEEDED(
        SQLGetStmtAttr(another_stmt, SQL_ATTR_IMP_ROW_DESC, &copy, 0, NULL)));
    EXPECT_EQ(SQL_ERROR, SQLCopyDesc(ard, copy));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_DESC, copy, SQLSTATE_CANNOT_MODIFY_IRD));
}

class TestSQLSetDescField : public testing::Test {
   public:
    void SetUp() {
        AllocStatement((SQLTCHAR*)(conn_string().c_str()), &m_env, &m_conn,
                       &m_hstmt, true, true);
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_ROW_DESC, &m_ard_hdesc, 0, NULL);
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_ROW_DESC, &m_ird_hdesc, 0, NULL);
    }

    void TearDown() {
        if (m_ard_hdesc != SQL_NULL_HDESC) {
            SQLFreeHandle(SQL_HANDLE_DESC, m_ard_hdesc);
        }
        if (m_ird_hdesc != SQL_NULL_HDESC) {
            SQLFreeHandle(SQL_HANDLE_DESC, m_ird_hdesc);
        }
        SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
        SQLDisconnect(m_conn);
        SQLFreeHandle(SQL_HANDLE_ENV, m_env);
    }

    SQLHENV m_env = SQL_NULL_HENV;
    SQLHDBC m_conn = SQL_NULL_HDBC;
    SQLHSTMT m_hstmt = SQL_NULL_HSTMT;
    SQLHDESC m_ard_hdesc = SQL_NULL_HDESC;
    SQLHDESC m_ird_hdesc = SQL_NULL_HDESC;
    SQLSMALLINT m_rec_number = 0;
    SQLSMALLINT m_field_identifier;
    SQLINTEGER m_buffer_length = SQL_NTS;
};

// Template for tests of SQLSetDescField
#define TEST_SQL_SET_DESC_FIELD(test_name, identifier, buffer_length, rec_num, \
                                value_ptr_assignment, expected_val, hdesc,     \
                                check_state)                                   \
    TEST_F(TestSQLSetDescField, test_name) {                                   \
        ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt),      \
                     &m_hstmt);                                                \
        m_field_identifier = identifier;                                       \
        m_buffer_length = buffer_length;                                       \
        m_rec_number = rec_num;                                                \
        value_ptr_assignment;                                                  \
        EXPECT_EQ(expected_val,                                                \
                  SQLSetDescField(hdesc, m_rec_number, m_field_identifier,     \
                                  (SQLPOINTER)m_value_ptr, m_buffer_length));  \
        if (check_state)                                                       \
            EXPECT_TRUE(                                                       \
                CheckSQLSTATE(SQL_HANDLE_DESC, hdesc,                          \
                              SQLSTATE_INVALID_DESCRIPTOR_FIELD_IDENTIFIER));  \
    }
#ifdef WIN32
#pragma warning(push)
// This warning detects an attempt to assign a 32-bit value to a 64-bit pointer
// type
#pragma warning(disable : 4312)
#elif defined(__APPLE__)
// This warning detects casts from integer types to void*.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
#elif defined(__linux__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif // WIN32 / __APPLE__/__linux__

// Descriptor Header Fields Tests
TEST_SQL_SET_DESC_FIELD(Test_SQL_DESC_ALLOC_TYPE, SQL_DESC_ALLOC_TYPE,
                        SQL_IS_SMALLINT, 0,
                        SQLSMALLINT m_value_ptr = SQL_DESC_ALLOC_USER;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(Test_SQL_DESC_ARRAY_SIZE, SQL_DESC_ARRAY_SIZE, SQL_NTS,
                        0, SQLULEN m_value_ptr = single_col_cnt;
                        , SQL_SUCCESS, m_ard_hdesc, 0);

TEST_SQL_SET_DESC_FIELD(Test_SQL_DESC_ARRAY_STATUS_PTR,
                        SQL_DESC_ARRAY_STATUS_PTR, SQL_NTS, 0, SQLUSMALLINT foo;
                        SQLUSMALLINT* m_value_ptr = &foo;
                        , SQL_SUCCESS, m_ard_hdesc, 0);

TEST_SQL_SET_DESC_FIELD(Test_SQL_DESC_BIND_OFFSET_PTR, SQL_DESC_BIND_OFFSET_PTR,
                        SQL_NTS, 0, SQLLEN foo;
                        SQLLEN* m_value_ptr = &foo;
                        , SQL_SUCCESS, m_ard_hdesc, 0);

TEST_SQL_SET_DESC_FIELD(Test_SQL_DESC_BIND_TYPE, SQL_DESC_BIND_TYPE, SQL_NTS, 0,
                        SQLINTEGER m_value_ptr = SQL_BIND_BY_COLUMN;
                        , SQL_SUCCESS, m_ard_hdesc, 0);

TEST_SQL_SET_DESC_FIELD(Test_SQL_DESC_COUNT, SQL_DESC_COUNT, SQL_IS_SMALLINT, 0,
                        SQLSMALLINT m_value_ptr = 25;
                        , SQL_SUCCESS, m_ard_hdesc, 0);

TEST_SQL_SET_DESC_FIELD(Test_SQL_DESC_ROWS_PROCESSED_PTR,
                        SQL_DESC_ROWS_PROCESSED_PTR, SQL_NTS, 0, SQLULEN foo;
                        SQLULEN* m_value_ptr = &foo;
                        , SQL_SUCCESS, m_ird_hdesc, 0);

// Descriptor Record Fields Tests
TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_AUTO_UNIQUE_VALUE,
                        SQL_DESC_AUTO_UNIQUE_VALUE, SQL_NTS, 1,
                        SQLINTEGER m_value_ptr = 0;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_BASE_COLUMN_NAME,
                        SQL_DESC_BASE_COLUMN_NAME, SQL_NTS, 1,
                        SQLCHAR m_value_ptr[255] = "_col2";
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_BASE_TABLE_NAME,
                        SQL_DESC_BASE_TABLE_NAME, SQL_NTS, 1,
                        SQLCHAR m_value_ptr[255] = "ODBCTest.Test";
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_CASE_SENSITIVE,
                        SQL_DESC_CASE_SENSITIVE, SQL_NTS, 1,
                        SQLINTEGER m_value_ptr = 1;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_CATALOG_NAME,
                        SQL_DESC_CATALOG_NAME, SQL_NTS, 1,
                        SQLCHAR m_value_ptr[255] = "";
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_CONCISE_TYPE,
                        SQL_DESC_CONCISE_TYPE, SQL_IS_INTEGER, 1,
                        SQLSMALLINT m_value_ptr = SQL_WLONGVARCHAR;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_DATA_PTR, SQL_DESC_DATA_PTR,
                        SQL_IS_POINTER, 1, SQLPOINTER m_value_ptr = NULL;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_DATETIME_INTERVAL_CODE,
                        SQL_DESC_DATETIME_INTERVAL_CODE, SQL_IS_SMALLINT, 1,
                        SQLSMALLINT m_value_ptr = 0;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_DATETIME_INTERVAL_PRECISION,
                        SQL_DESC_DATETIME_INTERVAL_PRECISION, SQL_IS_INTEGER, 1,
                        SQLINTEGER m_value_ptr = 0;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_DISPLAY_SIZE,
                        SQL_DESC_DISPLAY_SIZE, SQL_IS_POINTER, 1,
                        SQLLEN m_value_ptr = 32766;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_FIXED_PREC_SCALE,
                        SQL_DESC_FIXED_PREC_SCALE, SQL_IS_INTEGER, 1,
                        SQLSMALLINT m_value_ptr = 0;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_INDICATOR_PTR,
                        SQL_DESC_INDICATOR_PTR, SQL_IS_INTEGER, 1,
                        SQLLEN m_value_ptr = 0;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_LABEL, SQL_DESC_LABEL,
                        SQL_NTS, 1, SQLCHAR m_value_ptr[255] = "_col2";
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_LENGTH, SQL_DESC_LENGTH,
                        SQL_IS_INTEGER, 1, SQLULEN m_value_ptr = 32766;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_LITERAL_PREFIX,
                        SQL_DESC_LITERAL_PREFIX, SQL_NTS, 1,
                        SQLCHAR m_value_ptr[255] = "";
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_LITERAL_SUFFIX,
                        SQL_DESC_LITERAL_SUFFIX, SQL_NTS, 1,
                        SQLCHAR m_value_ptr[255] = "";
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_LOCAL_TYPE_NAME,
                        SQL_DESC_LOCAL_TYPE_NAME, SQL_NTS, 1,
                        SQLCHAR m_value_ptr[255] = "varchar";
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_NAME, SQL_DESC_NAME,
                        SQL_NTS, 1, SQLCHAR m_value_ptr[255] = "_col2";
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_NULLABLE, SQL_DESC_NULLABLE,
                        SQL_IS_SMALLINT, 1, SQLSMALLINT m_value_ptr = 1;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_NUM_PREC_RADIX,
                        SQL_DESC_NUM_PREC_RADIX, SQL_IS_INTEGER, 1,
                        SQLINTEGER m_value_ptr = 0;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_OCTET_LENGTH,
                        SQL_DESC_OCTET_LENGTH, SQL_NTS, 1,
                        SQLLEN m_value_ptr = 0;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_OCTET_LENGTH_PTR,
                        SQL_DESC_OCTET_LENGTH_PTR, SQL_IS_INTEGER, 1,
                        SQLLEN m_value_ptr = 0;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_PARAMETER_TYPE,
                        SQL_DESC_PARAMETER_TYPE, SQL_IS_SMALLINT, 1,
                        SQLSMALLINT m_value_ptr = 1;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_PRECISION,
                        SQL_DESC_PRECISION, SQL_IS_SMALLINT, 1,
                        SQLSMALLINT m_value_ptr = 30585;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_ROWVER, SQL_DESC_ROWVER,
                        SQL_NTS, 1, SQLSMALLINT m_value_ptr = 1;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_SCALE, SQL_DESC_SCALE,
                        SQL_IS_SMALLINT, 1, SQLSMALLINT m_value_ptr = 0;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_SCHEMA_NAME,
                        SQL_DESC_SCHEMA_NAME, SQL_NTS, 1,
                        SQLCHAR m_value_ptr[255] = "";
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_SEARCHABLE,
                        SQL_DESC_SEARCHABLE, SQL_IS_SMALLINT, 1,
                        SQLSMALLINT m_value_ptr = SQL_PRED_SEARCHABLE;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_TABLE_NAME,
                        SQL_DESC_TABLE_NAME, SQL_NTS, 1,
                        SQLCHAR m_value_ptr[255] = "ODBCTest.Test";
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_TYPE, SQL_DESC_TYPE,
                        SQL_IS_SMALLINT, 1,
                        SQLSMALLINT m_value_ptr = SQL_WLONGVARCHAR;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_TYPE_NAME,
                        SQL_DESC_TYPE_NAME, SQL_NTS, 1,
                        SQLCHAR m_value_ptr[255] = "varchar";
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_UNNAMED, SQL_DESC_UNNAMED,
                        SQL_IS_SMALLINT, 1, SQLSMALLINT m_value_ptr = SQL_NAMED;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_UNSIGNED, SQL_DESC_UNSIGNED,
                        SQL_IS_SMALLINT, 1, SQLSMALLINT m_value_ptr = SQL_TRUE;
                        , SQL_ERROR, m_ird_hdesc, 1);

TEST_SQL_SET_DESC_FIELD(TestUndefinedError_SQL_DESC_UPDATABLE,
                        SQL_DESC_UPDATABLE, SQL_IS_SMALLINT, 1,
                        SQLSMALLINT m_value_ptr = SQL_ATTR_READONLY;
                        , SQL_ERROR, m_ird_hdesc, 1);

class TestSQLGetDescField : public testing::Test {
   public:
    void SetUp() {
        AllocStatement((SQLTCHAR*)(conn_string().c_str()), &m_env, &m_conn,
                       &m_hstmt, true, true);
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_ROW_DESC, &m_ard_hdesc, 0, NULL);
        SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_ROW_DESC, &m_ird_hdesc, 0, NULL);
        ExecuteQuery(multi_col, data_set, convert_to_test_string(multi_row_cnt),
                     &m_hstmt);
    }

    void TearDown() {
        if (m_ard_hdesc != SQL_NULL_HDESC) {
            SQLFreeHandle(SQL_HANDLE_DESC, m_ard_hdesc);
        }
        if (m_ird_hdesc != SQL_NULL_HDESC) {
            SQLFreeHandle(SQL_HANDLE_DESC, m_ird_hdesc);
        }
        SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
        SQLDisconnect(m_conn);
        SQLFreeHandle(SQL_HANDLE_ENV, m_env);
    }

    SQLHENV m_env = SQL_NULL_HENV;
    SQLHDBC m_conn = SQL_NULL_HDBC;
    SQLHSTMT m_hstmt = SQL_NULL_HSTMT;
    SQLHDESC m_ard_hdesc = SQL_NULL_HDESC;
    SQLHDESC m_ird_hdesc = SQL_NULL_HDESC;
    SQLSMALLINT m_rec_number = 0;
    SQLINTEGER m_buffer_length = 0;
    SQLINTEGER m_string_length_ptr = 0;
};

// Descriptor Header Fields Tests
TEST_F(TestSQLGetDescField, Test_SQL_DESC_ALLOC_TYPE) {
    SQLSMALLINT m_value_ptr = 0;
    ASSERT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ird_hdesc, 0, SQL_DESC_ALLOC_TYPE,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)SQL_DESC_ALLOC_AUTO, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_ARRAY_SIZE) {
    SQLULEN m_value_ptr = 0;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ard_hdesc, 0, SQL_DESC_ARRAY_SIZE,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)1, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_ARRAY_STATUS_PTR) {
    SQLUSMALLINT* m_value_ptr;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ard_hdesc, 0, SQL_DESC_ARRAY_STATUS_PTR,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ(nullptr, m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_BIND_OFFSET_PTR) {
    SQLLEN* m_value_ptr;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ard_hdesc, 0, SQL_DESC_BIND_OFFSET_PTR,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ(nullptr, m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_BIND_TYPE) {
    SQLINTEGER m_value_ptr = 0;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ard_hdesc, 0, SQL_DESC_BIND_TYPE,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)SQL_BIND_BY_COLUMN, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_COUNT) {
    SQLSMALLINT m_value_ptr = 0;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ard_hdesc, 0, SQL_DESC_COUNT,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)multi_col_cnt, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_ROWS_PROCESSED_PTR) {
    SQLULEN* m_value_ptr = 0;
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(m_ird_hdesc, 0, SQL_DESC_ROWS_PROCESSED_PTR,
                        &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ(nullptr, m_value_ptr);
}

// Descriptor Record Fields Tests
TEST_F(TestSQLGetDescField, Test_SQL_DESC_AUTO_UNIQUE_VALUE) {
    SQLINTEGER m_value_ptr = 0;
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(m_ird_hdesc, 0, SQL_DESC_AUTO_UNIQUE_VALUE,
                        &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)FALSE, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_BASE_COLUMN_NAME) {
    SQLTCHAR m_value_ptr[255];
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(m_ird_hdesc, 1, SQL_DESC_BASE_COLUMN_NAME, &m_value_ptr,
                        255, &m_string_length_ptr)));
    EXPECT_EQ(tchar_to_string((SQLTCHAR*)CREATE_STRING("_col0")), tchar_to_string(m_value_ptr));
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_BASE_TABLE_NAME) {
    SQLTCHAR m_value_ptr[255];
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(m_ird_hdesc, 1, SQL_DESC_BASE_TABLE_NAME, &m_value_ptr,
                        255, &m_string_length_ptr)));
    EXPECT_EQ(tchar_to_string((SQLTCHAR*)CREATE_STRING("")), tchar_to_string(m_value_ptr));
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_CASE_SENSITIVE) {
    SQLINTEGER m_value_ptr;
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(m_ird_hdesc, 1, SQL_DESC_CASE_SENSITIVE, &m_value_ptr,
                        0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)SQL_TRUE, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_CATALOG_NAME) {
    SQLTCHAR m_value_ptr[255];
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(m_ird_hdesc, 1, SQL_DESC_CATALOG_NAME, &m_value_ptr,
                        255, &m_string_length_ptr)));
    EXPECT_EQ(tchar_to_string((SQLTCHAR*)CREATE_STRING("")), tchar_to_string(m_value_ptr));
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_CONCISE_TYPE) {
    SQLSMALLINT m_value_ptr;
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(m_ird_hdesc, 6, SQL_DESC_CONCISE_TYPE, &m_value_ptr,
                        0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)SQL_TYPE_DATE, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_DATA_PTR) {
    SQLPOINTER m_value_ptr;
    EXPECT_TRUE(SQL_SUCCEEDED(SQLGetDescField(m_ard_hdesc, 1, SQL_DESC_DATA_PTR,
                                              &m_value_ptr,
                        0, &m_string_length_ptr)));
    EXPECT_EQ(nullptr, m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_DATETIME_INTERVAL_CODE) {
    DATE_STRUCT date;
    SQLLEN indicator = 0;
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLBindCol(m_hstmt, 6, SQL_C_TYPE_DATE, &date, 0, &indicator)));
    SQLSMALLINT m_value_ptr;
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(m_ard_hdesc, 6, SQL_DESC_DATETIME_INTERVAL_CODE,
                        &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)SQL_CODE_DATE, (uint64_t)m_value_ptr);
}

// This field contains the interval leading precision if the SQL_DESC_TYPE field
// is SQL_INTERVAL. As SQL_INTERVAL support is disabled because some
// applications are unhappy with it, this test should return SQL_ERROR as
// DESC_INVALID_DESCRIPTOR_IDENTIFIER
TEST_F(TestSQLGetDescField, Test_SQL_DESC_DATETIME_INTERVAL_PRECISION) {
    SQLINTEGER m_value_ptr = 0;
    EXPECT_EQ(
        SQL_ERROR,
        SQLGetDescField(m_ard_hdesc, 7, SQL_DESC_DATETIME_INTERVAL_PRECISION,
                        &m_value_ptr, 0, &m_string_length_ptr));
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_DISPLAY_SIZE) {
    SQLLEN m_value_ptr;
    // Column 4 with type Double
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ird_hdesc, 4, SQL_DESC_DISPLAY_SIZE,
                        &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)24, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_FIXED_PREC_SCALE) {
    SQLSMALLINT m_value_ptr;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ird_hdesc, 4, SQL_DESC_FIXED_PREC_SCALE,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)SQL_FALSE, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_INDICATOR_PTR) {
    SQLLEN* m_value_ptr;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ard_hdesc, 4, SQL_DESC_INDICATOR_PTR,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ(nullptr, m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_LABEL) {
    SQLTCHAR m_value_ptr[255];
    // Column 1 with name "_col0"
    EXPECT_TRUE(SQL_SUCCEEDED(SQLGetDescField(m_ird_hdesc, 1, SQL_DESC_LABEL,
                                              &m_value_ptr, 255,
                                              &m_string_length_ptr)));
    EXPECT_EQ(tchar_to_string((SQLTCHAR*)CREATE_STRING("_col0")), tchar_to_string(m_value_ptr));
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_LENGTH) {
    SQLULEN m_value_ptr;
    // Column 6 with type Date
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ird_hdesc, 6, SQL_DESC_LENGTH,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t) 10, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_LITERAL_PREFIX) {
    SQLTCHAR m_value_ptr[255];
    // Column 4 with type Double
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(m_ird_hdesc, 4, SQL_DESC_LITERAL_PREFIX,
                                              &m_value_ptr, 255,
                                              &m_string_length_ptr)));
    EXPECT_EQ(tchar_to_string((SQLTCHAR*)CREATE_STRING("DOUBLE '")), tchar_to_string(m_value_ptr));
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_LITERAL_SUFFIX) {
    SQLTCHAR m_value_ptr[255];
    // Column 4 with type Double
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(m_ird_hdesc, 4, SQL_DESC_LITERAL_SUFFIX, &m_value_ptr,
                        255, &m_string_length_ptr)));
    EXPECT_EQ(tchar_to_string((SQLTCHAR*)CREATE_STRING("'")), tchar_to_string(m_value_ptr));
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_LOCAL_TYPE_NAME) {
    SQLTCHAR m_value_ptr[255];
    // Column 4 with type Double
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(m_ird_hdesc, 4, SQL_DESC_LOCAL_TYPE_NAME, &m_value_ptr,
                        255, &m_string_length_ptr)));
    EXPECT_EQ(tchar_to_string((SQLTCHAR*)CREATE_STRING("double")), tchar_to_string(m_value_ptr));
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_NAME) {
    SQLTCHAR m_value_ptr[255];
    // Column 1 with name "_col0"
    EXPECT_TRUE(SQL_SUCCEEDED(SQLGetDescField(m_ird_hdesc, 1, SQL_DESC_NAME,
                                              &m_value_ptr,
                        255, &m_string_length_ptr)));
    EXPECT_EQ(tchar_to_string((SQLTCHAR*)CREATE_STRING("_col0")), tchar_to_string(m_value_ptr));
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_NULLABLE) {
    SQLSMALLINT m_value_ptr;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ird_hdesc, 1, SQL_DESC_NULLABLE,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)SQL_NULLABLE, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_NUM_PREC_RADIX) {
    SQLINTEGER m_value_ptr;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ard_hdesc, 1, SQL_DESC_NUM_PREC_RADIX,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)10, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_OCTET_LENGTH) {
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLLEN buf_len = 1;
    SQLUSMALLINT col = 2;
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLBindCol(m_hstmt, col, SQL_C_CHAR, &data, buf_len, &indicator)));
    SQLLEN m_value_ptr = 0;
    // Column 2 with type Varchar
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ard_hdesc, col, SQL_DESC_OCTET_LENGTH,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)buf_len, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_OCTET_LENGTH_PTR) {
    SQLLEN* m_value_ptr;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ard_hdesc, 1, SQL_DESC_OCTET_LENGTH_PTR,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ(nullptr, m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_PRECISION) {
    DATE_STRUCT date;
    SQLLEN indicator = 0;
    SQLUSMALLINT col = 7;
    EXPECT_TRUE(SQL_SUCCEEDED(SQLBindCol(
        m_hstmt, col, SQL_C_INTERVAL_DAY_TO_SECOND, &date, 0, &indicator)));
    SQLSMALLINT m_value_ptr = 0;
    // Column 7 with type interval day to second
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ard_hdesc, col, SQL_DESC_PRECISION,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)6, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_ROWVER) {
    SQLSMALLINT m_value_ptr;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ird_hdesc, 1, SQL_DESC_ROWVER,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)SQL_FALSE, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_SCALE) {
    SQLSMALLINT m_value_ptr;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ird_hdesc, 1, SQL_DESC_SCALE,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)0, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_SCHEMA_NAME) {
    SQLTCHAR m_value_ptr[255];
    // Column 4 with type Double
    EXPECT_TRUE(SQL_SUCCEEDED(
        SQLGetDescField(m_ird_hdesc, 4, SQL_DESC_SCHEMA_NAME, &m_value_ptr,
                        255, &m_string_length_ptr)));
    EXPECT_EQ(tchar_to_string((SQLTCHAR*)CREATE_STRING("")), tchar_to_string(m_value_ptr));
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_SEARCHABLE) {
    SQLSMALLINT m_value_ptr;
    EXPECT_TRUE(
        SQL_SUCCEEDED(SQLGetDescField(m_ird_hdesc, 1, SQL_DESC_SEARCHABLE,
                                      &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)SQL_SEARCHABLE, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_TABLE_NAME) {
    SQLTCHAR m_value_ptr[255];
    EXPECT_TRUE(SQL_SUCCEEDED(SQLGetDescField(m_ird_hdesc, 1,
                                              SQL_DESC_TABLE_NAME, &m_value_ptr,
                                              255, &m_string_length_ptr)));
    EXPECT_EQ(tchar_to_string((SQLTCHAR*)CREATE_STRING("")), tchar_to_string(m_value_ptr));
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_TYPE) {
    SQLSMALLINT m_value_ptr = 0;
    // Column 6 with type Date
    EXPECT_TRUE(SQL_SUCCEEDED(SQLGetDescField(
        m_ird_hdesc, 6, SQL_DESC_TYPE, &m_value_ptr, 0, &m_string_length_ptr)));
    EXPECT_EQ((uint64_t)SQL_DATETIME, (uint64_t)m_value_ptr);
}

TEST_F(TestSQLGetDescField, Test_SQL_DESC_TYPE_NAME) {
    SQLTCHAR m_value_ptr[255];
    // Column 6 with type Date
    EXPECT_TRUE(SQL_SUCCEEDED(SQLGetDescField(m_ird_hdesc, 6,
                                              SQL_DESC_TYPE_NAME, &m_value_ptr,
                                              255, &m_string_length_ptr)));
    EXPECT_EQ(tchar_to_string((SQLTCHAR*)CREATE_STRING("date")), tchar_to_string(m_value_ptr));
}
TEST_F(TestSQLSetDescRec, APP_PARAM_SET) {
    SQLHDESC hdesc;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_PARAM_DESC, &hdesc, 0, NULL);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER data = 12345;
    ret = SQLSetDescRec(hdesc, 1, SQL_INTEGER, 0, 0, 0, 0, (SQLPOINTER)&data,
                        NULL, NULL);
    EXPECT_EQ(SQL_ERROR, ret);
}

TEST_F(TestSQLSetDescRec, APP_ROW_SET) {
    SQLHDESC hdesc;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_ROW_DESC, &hdesc, 0, NULL);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER data = 12345;
    ret = SQLSetDescRec(hdesc, 1, SQL_INTEGER, 0, 0, 0, 0, (SQLPOINTER)&data,
                        NULL, NULL);
    EXPECT_EQ(SQL_ERROR, ret);
}

TEST_F(TestSQLSetDescRec, IMP_PARAM_SET) {
    SQLHDESC hdesc;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_PARAM_DESC, &hdesc, 0, NULL);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER data = 12345;
    ret = SQLSetDescRec(hdesc, 1, SQL_INTEGER, 0, 0, 0, 0, (SQLPOINTER)&data,
                        NULL, NULL);
    EXPECT_EQ(SQL_ERROR, ret);
}

TEST_F(TestSQLSetDescRec, IMP_ROW_SET) {
    SQLHDESC hdesc;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_ROW_DESC, &hdesc, 0, NULL);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER data = 12345;
    ret = SQLSetDescRec(hdesc, 1, SQL_INTEGER, 0, 0, 0, 0, (SQLPOINTER)&data,
                        NULL, NULL);
    EXPECT_EQ(SQL_ERROR, ret);
}

TEST_F(TestSQLGetDescRec, APP_PARAM_GET) {
    SQLHDESC hdesc;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_PARAM_DESC, &hdesc, 0, NULL);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    test_string column_name = CREATE_STRING("column_name");
    ret = SQLGetDescRec(hdesc, 1, (SQLTCHAR*)column_name.c_str(),
                        (SQLSMALLINT)column_name.size(), NULL, NULL, NULL, NULL,
                        NULL, NULL, NULL);
    EXPECT_EQ(SQL_ERROR, ret);
}

TEST_F(TestSQLGetDescRec, APP_ROW_GET) {
    SQLHDESC hdesc;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetStmtAttr(m_hstmt, SQL_ATTR_APP_ROW_DESC, &hdesc, 0, NULL);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    test_string column_name = CREATE_STRING("column_name");
    ret = SQLGetDescRec(hdesc, 1, (SQLTCHAR*)column_name.c_str(),
                        (SQLSMALLINT)column_name.size(), NULL, NULL, NULL, NULL,
                        NULL, NULL, NULL);
    EXPECT_EQ(SQL_ERROR, ret);
}

TEST_F(TestSQLGetDescRec, IMP_PARAM_GET) {
    SQLHDESC hdesc;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_PARAM_DESC, &hdesc, 0, NULL);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    test_string column_name = CREATE_STRING("column_name");
    ret = SQLGetDescRec(hdesc, 1, (SQLTCHAR*)column_name.c_str(),
                        (SQLSMALLINT)column_name.size(), NULL, NULL, NULL, NULL,
                        NULL, NULL, NULL);
    EXPECT_EQ(SQL_ERROR, ret);
}

TEST_F(TestSQLGetDescRec, IMP_ROW_GET) {
    SQLHDESC hdesc;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetStmtAttr(m_hstmt, SQL_ATTR_IMP_ROW_DESC, &hdesc, 0, NULL);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    test_string column_name = CREATE_STRING("column_name");
    ret = SQLGetDescRec(hdesc, 1, (SQLTCHAR*)column_name.c_str(),
                             (SQLSMALLINT)column_name.size(), NULL, NULL, NULL,
                             NULL, NULL, NULL, NULL);
    EXPECT_EQ(SQL_ERROR, ret);
}

#ifdef WIN32
#pragma warning(pop)
#elif __APPLE__
#pragma clang diagnostic pop
#elif defined(__linux__)
#pragma GCC diagnostic pop
#endif // WIN32 / __APPLE__ / __linux__

int main(int argc, char** argv) {
#ifdef WIN32
    // Enable CRT for detecting memory leaks
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#ifdef __APPLE__
    // Enable malloc logging for detecting memory leaks.
    system("export MallocStackLogging=1");
#endif
    testing::internal::CaptureStdout();
    ::testing::InitGoogleTest(&argc, argv);

    int failures = RUN_ALL_TESTS();

    std::string output = testing::internal::GetCapturedStdout();
    std::cout << output << std::endl;
    std::cout << (failures ? "Not all tests passed." : "All tests passed")
              << std::endl;
    WriteFileIfSpecified(argv, argv + argc, "-fout", output);

#ifdef __APPLE__
    // Disable malloc logging and report memory leaks
    system("unset MallocStackLogging");
    system("leaks itodbc_descriptors > leaks_itodbc_descriptors");
#endif
    return failures;
}
