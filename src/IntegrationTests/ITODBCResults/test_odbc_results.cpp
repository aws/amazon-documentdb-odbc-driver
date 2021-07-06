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
#include <cmath>
#include <time.h>
// clang-format on

const test_string table_name = CREATE_STRING("ODBCTest.IoT");
//const test_string multi_type_data_set = CREATE_STRING("kibana_sample_data_types");
const test_string single_col = CREATE_STRING("time");
const test_string single_row = CREATE_STRING("1");
const size_t single_row_cnt = 1;
const size_t multi_row_cnt = 2;
const size_t multi_col_cnt = 5;
const size_t single_col_cnt = 1;
const test_string multi_col = CREATE_STRING("null, 1, VARCHAR '2', DOUBLE '3.3', true");
const size_t buffer_size = 255;

inline void BindColumns(std::vector< std::vector< char > >& cols,
                        std::vector< std::vector< SQLLEN > >& lens,
                        SQLHSTMT* hstmt) {
    SQLRETURN ret;
    for (size_t i = 0; i < cols.size(); i++) {
        ret = SQLBindCol(*hstmt, (SQLUSMALLINT)i + 1, SQL_C_CHAR,
                         &cols[i][0], buffer_size,
                         &lens[i][0]);
        LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
        ASSERT_TRUE(SQL_SUCCEEDED(ret));
    }
}

void ExecuteQuery(const test_string& column,
                         const test_string& table, const test_string& count,
                         SQLHSTMT* hstmt) {
    test_string statement = QueryBuilder(column, table, count);
    SQLRETURN ret = SQLExecDirect(*hstmt, (SQLTCHAR*)statement.c_str(),
                                  (SQLINTEGER)statement.length());
    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
}

void BindColSetup(const size_t row_cnt, const size_t row_fetch_cnt,
                  const test_string& column_name,
                  std::vector< std::vector< char > >& cols, 
                  std::vector< std::vector< SQLLEN > >& lens, SQLHSTMT* hstmt) {
    SQLRETURN ret =
        SQLSetStmtAttr(*hstmt, SQL_ROWSET_SIZE, (void*)row_fetch_cnt, 0);
    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
    ASSERT_EQ(ret, SQL_SUCCESS);

    test_string row_str = convert_to_test_string(row_cnt);
    ExecuteQuery(column_name, table_name, row_str, hstmt);

    for (size_t i = 0; i < cols.size(); i++) {
        cols[i].resize(row_fetch_cnt * buffer_size);
        lens[i].resize(row_fetch_cnt);
    }
}

inline void QueryBind(const size_t row_cnt, const size_t row_fetch_cnt,
                      const test_string& column_name,
                      std::vector< std::vector< char > >& cols,
                      std::vector< std::vector< SQLLEN > >& lens,
                      SQLHSTMT* hstmt) {
    BindColSetup(row_cnt, row_fetch_cnt, column_name, cols, lens, hstmt);
    BindColumns(cols, lens, hstmt);
}

void QueryFetch(const test_string& column, const test_string& dataset,
                const test_string& count, SQLHSTMT* hstmt) {
    ExecuteQuery(column, dataset, count, hstmt);
    SQLRETURN ret = SQLFetch(*hstmt);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
}

void CheckIndicatorRet(const test_string& expected, const SQLLEN indicator, const SQLRETURN ret, const bool is_wchar) {
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    if (is_wchar) {
#ifdef __APPLE__
        ASSERT_EQ((long)(4 * expected.size()), (long)indicator);
#else
        ASSERT_EQ((long)(2 * expected.size()), (long)indicator);
#endif
    } else {
        ASSERT_EQ((long)(expected.size()), (long)indicator);
    }
}

void CompareData(const test_string& expected, const SQLLEN indicator, const SQLWCHAR* data, const SQLRETURN ret) {
    CheckIndicatorRet(expected, indicator, ret, true);
    EXPECT_EQ(expected, to_test_string(wchar_to_string(data)));
}

void CompareData(const test_string& expected, const SQLLEN indicator, const SQLCHAR* data, const SQLRETURN ret) {
    CheckIndicatorRet(expected, indicator, ret, false);
    EXPECT_EQ(expected, to_test_string(std::string((char*)data)));
}

auto CompareTimestampStruct = [](const TIMESTAMP_STRUCT& expected,
                                 const TIMESTAMP_STRUCT& actual) {
    EXPECT_EQ(expected.year, actual.year);
    EXPECT_EQ(expected.month, actual.month);
    EXPECT_EQ(expected.day, actual.day);
    EXPECT_EQ(expected.hour, actual.hour);
    EXPECT_EQ(expected.minute, actual.minute);
    EXPECT_EQ(expected.second, actual.second);
    EXPECT_EQ(expected.fraction, actual.fraction);
};

auto CompareDateStruct = [](const DATE_STRUCT& expected,
                            const DATE_STRUCT& actual) {
    EXPECT_EQ(expected.year, actual.year);
    EXPECT_EQ(expected.month, actual.month);
    EXPECT_EQ(expected.day, actual.day);
};

auto CompareTimeStruct = [](const TIME_STRUCT& expected,
                            const TIME_STRUCT& actual) {
    EXPECT_EQ(expected.hour, actual.hour);
    EXPECT_EQ(expected.minute, actual.minute);
    EXPECT_EQ(expected.second, actual.second);
};

auto CompareIntervalStruct = [](const SQL_INTERVAL_STRUCT& expected,
                                const SQL_INTERVAL_STRUCT& actual) {
    ASSERT_TRUE(actual.interval_type == SQL_IS_YEAR_TO_MONTH
                || actual.interval_type == SQL_IS_DAY_TO_SECOND);
    ASSERT_EQ(expected.interval_type, actual.interval_type);
    EXPECT_EQ(expected.interval_sign, actual.interval_sign);
    if (expected.interval_type == SQL_IS_YEAR_TO_MONTH) {
        EXPECT_EQ(expected.intval.year_month.month,
                  actual.intval.year_month.month);
        EXPECT_EQ(expected.intval.year_month.year,
                  actual.intval.year_month.year);
    } else if (expected.interval_type == SQL_IS_DAY_TO_SECOND) {
        EXPECT_EQ(expected.intval.day_second.day, actual.intval.day_second.day);
        EXPECT_EQ(expected.intval.day_second.hour, actual.intval.day_second.hour);
        EXPECT_EQ(expected.intval.day_second.minute, actual.intval.day_second.minute);
        EXPECT_EQ(expected.intval.day_second.second, actual.intval.day_second.second);
        EXPECT_EQ(expected.intval.day_second.fraction, actual.intval.day_second.fraction);
    } else {
        FAIL();
    }
};

auto ConstructIntervalStruct =
    [](SQLINTERVAL type, SQLSMALLINT sign,
       SQLUINTEGER year, SQLUINTEGER month, SQLUINTEGER day, SQLUINTEGER hour,
       SQLUINTEGER minute, SQLUINTEGER second, SQLUINTEGER fraction) {
        SQL_INTERVAL_STRUCT st;
        st.interval_type = type;
        st.interval_sign = sign;
        if (SQL_IS_YEAR_TO_MONTH == type) {
            st.intval.year_month.year = year;
            st.intval.year_month.month = month;
        } else if (SQL_IS_DAY_TO_SECOND == type) {
            st.intval.day_second.day = day;
            st.intval.day_second.hour = hour;
            st.intval.day_second.minute = minute;
            st.intval.day_second.second = second;
            st.intval.day_second.fraction = fraction;
        }
        return st;
    };

template < class T >
void TypeConversionAssertionTemplate(
    SQLHSTMT& hstmt, SQLSMALLINT type, const test_string& columns,
    const std::vector< std::pair< T, SQLTCHAR* > >& expected,
    void (*compare_func)(const T&, const T&)) {
    QueryFetch(columns, table_name, single_row, &hstmt);
    T actual;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    for (size_t i = 0; i < expected.size(); ++i) {
        ret = SQLGetData(hstmt, static_cast< SQLUSMALLINT >(i + 1), type,
                         &actual, sizeof(actual), &indicator);
        LogAnyDiagnostics(SQL_HANDLE_STMT, hstmt, ret);
        SQLTCHAR* expected_state = expected[i].second;
        // Success and no error code
        if (!expected_state) {
            EXPECT_EQ(SQL_SUCCESS, ret);
            EXPECT_EQ((SQLLEN)sizeof(T), indicator);
            compare_func(expected[i].first, actual);
        }
        // Success with info and error code
        else if (tchar_to_string(expected_state) == tchar_to_string(SQLSTATE_FRACTIONAL_TRUNCATION)) {
            EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
            EXPECT_EQ((SQLLEN)sizeof(T), indicator);
            compare_func(expected[i].first, actual);
            EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, hstmt,
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
        }
        // Error and error code
        else if (tchar_to_string(expected_state) == tchar_to_string(SQLSTATE_STRING_CONVERSION_ERROR)) {
            EXPECT_EQ(SQL_ERROR, ret);
            EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, hstmt,
                                      SQLSTATE_STRING_CONVERSION_ERROR));
        } else if (tchar_to_string(expected_state) == tchar_to_string(SQLSTATE_RESTRICTED_DATA_TYPE_ERROR)) {
            EXPECT_EQ(SQL_ERROR, ret);
            EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, hstmt,
                                      SQLSTATE_RESTRICTED_DATA_TYPE_ERROR));
        }
        // Unknown SQL state
        else {
            FAIL();
        }
    }
    ASSERT_NO_THROW(CloseCursor(&hstmt, true, true));
}

auto TestConvertingToDate =
    [](SQLHSTMT& hstmt, const test_string& columns,
       const std::vector< std::pair< DATE_STRUCT, SQLTCHAR* > >& expected) {
        TypeConversionAssertionTemplate< DATE_STRUCT >(
            hstmt, SQL_C_TYPE_DATE, columns, expected, CompareDateStruct);
        TypeConversionAssertionTemplate< DATE_STRUCT >(
            hstmt, SQL_C_DATE, columns, expected, CompareDateStruct);
    };

auto TestConvertingToTime =
    [](SQLHSTMT& hstmt, const test_string& columns,
       const std::vector< std::pair< TIME_STRUCT, SQLTCHAR* > >& expected) {
        TypeConversionAssertionTemplate< TIME_STRUCT >(
            hstmt, SQL_C_TYPE_TIME, columns, expected, CompareTimeStruct);
        TypeConversionAssertionTemplate< TIME_STRUCT >(
            hstmt, SQL_C_TIME, columns, expected, CompareTimeStruct);
    };

auto TestConvertingToTimestamp =
    [](SQLHSTMT& hstmt, const test_string& columns,
       const std::vector< std::pair< TIMESTAMP_STRUCT, SQLTCHAR* > >&
           expected) {
        TypeConversionAssertionTemplate< TIMESTAMP_STRUCT >(
            hstmt, SQL_C_TYPE_TIMESTAMP, columns, expected,
            CompareTimestampStruct);
        TypeConversionAssertionTemplate< TIMESTAMP_STRUCT >(
            hstmt, SQL_C_TIMESTAMP, columns, expected, CompareTimestampStruct);
    };

class TestSQLGetData : public Fixture {};

class TestSQLNumResultCols : public Fixture {};

class TestSQLMoreResults : public Fixture {};

class TestSQLDescribeCol : public Fixture {};

class TestSQLRowCount : public Fixture {};

class TestSQLBindCol : public Fixture {};

TEST_F(TestSQLBindCol, SingleColumnSingleBind) {
    std::vector< std::vector< char > > cols(single_col_cnt);
    std::vector< std::vector< SQLLEN > > lens(single_col_cnt);
    QueryBind(single_row_cnt, 1, single_col, cols, lens, &m_hstmt);
}

TEST_F(TestSQLBindCol, MultiColumnsMultiBind) {
    std::vector< std::vector< char > > cols(multi_col_cnt);
    std::vector< std::vector< SQLLEN > > lens(multi_col_cnt);
    QueryBind(single_row_cnt, 1, multi_col, cols, lens, &m_hstmt);
}

TEST_F(TestSQLBindCol, MultiColumnsMultiRows) {
    std::vector< std::vector< char > > cols(multi_col_cnt);
    std::vector< std::vector< SQLLEN > > lens(multi_col_cnt);
    QueryBind(multi_row_cnt, multi_row_cnt, multi_col, cols, lens, &m_hstmt);
}

// Looked at SQLBindCol - if < requested column are allocated, it will
// reallocate additional space for that column
TEST_F(TestSQLBindCol, InvalidColIndex0) {
    std::vector< std::vector< char > > cols(single_col_cnt);
    std::vector< std::vector< SQLLEN > > lens(single_col_cnt);
    BindColSetup(single_row_cnt, 1, single_col, cols, lens, &m_hstmt);
    SQLRETURN ret = SQLBindCol(m_hstmt, (SQLUSMALLINT)1, SQL_C_CHAR,
                     &cols[0][0], buffer_size,
                     &lens[0][0]);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(m_hstmt, (SQLUSMALLINT)0, SQL_C_CHAR,
                     &cols[0][0], buffer_size,
                     &lens[0][0]);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_RESTRICTED_DATA_TYPE_ERROR));
}

TEST_F(TestSQLBindCol, InvalidColIndex2) {
    std::vector< std::vector< char > > cols(2);
    std::vector< std::vector< SQLLEN > > lens(2);
    BindColSetup(single_row_cnt, 1, single_col, cols, lens, &m_hstmt);
    SQLRETURN ret = SQLBindCol(m_hstmt, (SQLUSMALLINT)1, SQL_C_CHAR,
                     &cols[0][0], buffer_size,
                     &lens[0][0]);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(m_hstmt, (SQLUSMALLINT)2, SQL_C_CHAR,
                     &cols[1][0], buffer_size,
                     &lens[1][0]);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_INVALID_DESCRIPTOR_INDEX));
}

TEST_F(TestSQLBindCol, InvalidBufferLength) {
    std::vector< std::vector< char > > cols(single_col_cnt);
    std::vector< std::vector< SQLLEN > > lens(single_col_cnt);
    BindColSetup(single_row_cnt, 1, single_col, cols, lens, &m_hstmt);
    SQLRETURN ret = SQLBindCol(m_hstmt, (SQLUSMALLINT)1, SQL_C_CHAR,
                     &cols[0][0], -1,
                     &lens[0][0]);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_INVALID_STRING_OR_BUFFER_LENGTH));
}

TEST_F(TestSQLBindCol, InsufficientSpace) {
    SQLRETURN ret = SQLSetStmtAttr(m_hstmt, SQL_ROWSET_SIZE, (void*)1, 0);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    ASSERT_EQ(ret, SQL_SUCCESS);

    test_string row_str = convert_to_test_string(single_row_cnt);
    test_string col = CREATE_STRING("VARCHAR '12345'");
    test_string colret = CREATE_STRING("1");
    ExecuteQuery(col, table_name, row_str, &m_hstmt);

    SQLLEN length = 0;
    std::vector< SQLTCHAR > data_buffer(2);
    ret = SQLBindCol(m_hstmt, (SQLUSMALLINT)1, SQL_C_CHAR,
                     (SQLPOINTER)data_buffer.data(), 2, &length);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));

    SQLULEN row_cnt = 0;
    SQLUSMALLINT row_stat = 0;
    std::vector< SQLTCHAR > msg_buffer(512);
    ret = SQLExtendedFetch(m_hstmt, SQL_FETCH_NEXT, 0, &row_cnt, &row_stat);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret, msg_buffer.data(), 512);
    EXPECT_EQ(ret, SQL_SUCCESS_WITH_INFO);
    EXPECT_EQ(tchar_to_string(msg_buffer.data()), tchar_to_string((SQLTCHAR*)CREATE_STRING("Fetched item was truncated.")));
    EXPECT_EQ(tchar_to_string(data_buffer.data()), tchar_to_string((SQLTCHAR*)colret.c_str()));
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_BIT) {
    int v1 = 0;
    int v2 = 1;
    int v3 = -1;  // underflow
    int v4 = 2;  // overflow
    test_string columns = CREATE_STRING("INTEGER\'") + convert_to_test_string(v1) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v1 = static_cast< SQLCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v2 = static_cast< SQLCHAR >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_STINYINT) {
    int v1 = SCHAR_MIN;
    int v2 = SCHAR_MAX;
    int v3 = SCHAR_MIN - 1;  // underflow
    int v4 = SCHAR_MAX + 1;      // overflow
    test_string columns = CREATE_STRING("INTEGER\'") + convert_to_test_string(v1) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v1 = static_cast< SQLSCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v2 = static_cast< SQLSCHAR >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_TINYINT) {
    int v1 = SCHAR_MIN;
    int v2 = SCHAR_MAX;
    int v3 = INT_MIN;   // underflow
    int v4 = INT_MAX;  // overflow
    test_string columns = CREATE_STRING("INTEGER\'") + convert_to_test_string(v1) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v1 = static_cast< SQLSCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v2 = static_cast< SQLSCHAR >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_UTINYINT) {
    int v1 = UCHAR_MAX;
    int v2 = -1;  // underflow
    int v3 = UCHAR_MAX + 1;  // overflow
    test_string columns = CREATE_STRING("INTEGER\'") + to_test_string(std::to_string(v1)) + CREATE_STRING("\', INTEGER\'")
                           + to_test_string(std::to_string(v2)) + CREATE_STRING("\', INTEGER\'")
                           + to_test_string(std::to_string(v3)) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v1 = static_cast< SQLCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 3, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_SLONG) {
    int v1 = -293719;
    int v2 = 741370;
    test_string columns = CREATE_STRING("INTEGER\'") + convert_to_test_string(v1) + CREATE_STRING("\', INTEGER\'") + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SLONG , &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v1 = static_cast< SQLINTEGER >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SLONG , &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v2 = static_cast< SQLINTEGER >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_LONG) {
    int v1 = -293719;
    int v2 = 741370;
    test_string columns = CREATE_STRING("INTEGER\'") + convert_to_test_string(v1) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v1 = static_cast< SQLINTEGER >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v2 = static_cast< SQLINTEGER >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_ULONG) {
    int v1 = 293719;
    int v2 = -1;  // underflow
    test_string columns = CREATE_STRING("INTEGER\'") + convert_to_test_string(v1) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUINTEGER expected_v1 = static_cast< SQLUINTEGER >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLUINTEGER), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_SSHORT) {
    int v1 = SHRT_MIN;
    int v2 = SHRT_MAX;
    int v3 = SHRT_MIN - 1;  // underflow
    int v4 = SHRT_MAX + 1;  // overflow
    test_string columns = CREATE_STRING("INTEGER\'") + convert_to_test_string(v1) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v1 = static_cast< SQLSMALLINT >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v2 = static_cast< SQLSMALLINT >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_SHORT) {
    int v1 = SHRT_MIN;
    int v2 = SHRT_MAX;
    int v3 = INT_MIN;  // underflow
    int v4 = INT_MAX;  // overflow
    test_string columns = CREATE_STRING("INTEGER\'") + convert_to_test_string(v1) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v1 = static_cast< SQLSMALLINT >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v2 = static_cast< SQLSMALLINT >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_USHORT) {
    int v1 = USHRT_MAX;
    int v2 = -1;  // underflow
    int v3 = USHRT_MAX + 1;  // overflow
    test_string columns = CREATE_STRING("INTEGER\'") + convert_to_test_string(v1) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUSMALLINT expected_v1 = static_cast< SQLUSMALLINT >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLUSMALLINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 3, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_SBIGINT) {
    int v1 = -293719;
    int v2 = 741370;
    test_string columns = CREATE_STRING("INTEGER\'") + convert_to_test_string(v1) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLBIGINT expected_v1 = static_cast< SQLBIGINT >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLBIGINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLBIGINT expected_v2 = static_cast< SQLBIGINT >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLBIGINT), indicator);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_UBIGINT) {
    int v1 = 293719;
    int v2 = -1;  // underflow
    test_string columns = CREATE_STRING("INTEGER\'") + convert_to_test_string(v1) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUBIGINT expected_v1 = static_cast< SQLUBIGINT >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLUBIGINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_CHAR) {
    int v1 = -293719;
    int v2 = 741370;
    test_string columns = CREATE_STRING("INTEGER\'") + convert_to_test_string(v1) + CREATE_STRING("\', INTEGER\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLCHAR data2[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_v1 = std::to_string(v1);
    ASSERT_EQ((SQLLEN)expected_v1.size(), indicator);
    EXPECT_STREQ(expected_v1.c_str(), (char*)data);
    SQLLEN expected_size = std::to_string(v2).size();
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data2, expected_size, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_DATA_RIGHT_TRUNCATED));
    ASSERT_EQ(expected_size, indicator);
    char expected_v2[1024] = {0};
    strncpy(expected_v2, std::to_string(v2).c_str(),
            static_cast< size_t >(expected_size - 1));
    EXPECT_STREQ(expected_v2, (char*)data2);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_BIT) {
    double v1 = 0.0;
    double v2 = 1.0;
    double v3 = -1.0;  // underflow
    double v4 = 2.0;   // overflow
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v1 = static_cast< SQLCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v2 = static_cast< SQLCHAR >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_STINYINT) {
    double v1 = -3.9E-5;
    double v2 = 3.9E-5;
    double v3 = -1.29E2;  // underflow
    double v4 = 1.28E2;  // overflow     
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v1 = static_cast< SQLSCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v2 = static_cast< SQLSCHAR >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_TINYINT) {
    double v1 = -1.279E2;
    double v2 = 1.269E2;
    double v3 = LLONG_MIN;  // underflow
    double v4 = ULONG_MAX;  // overflow
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v1 = static_cast< SQLSCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v2 = static_cast< SQLSCHAR >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_UTINYINT) {
    double v1 = 1.1;
    double v2 = -3.0; // underflow
    double v3 = 2.56E2;  // overflow
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v1 = static_cast< SQLCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 3, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_SLONG) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    double v3 = -9.3E18;  // underflow
    double v4 = (double)LONG_MAX + (double)1;  // overflow  
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v1 = static_cast< SQLINTEGER >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v2 = static_cast< SQLINTEGER >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_LONG) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    double v3 = -DBL_MAX;  // underflow
    double v4 = DBL_MAX;  // overflow
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v1 = static_cast< SQLINTEGER >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v2 = static_cast< SQLINTEGER >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_ULONG) {
    double v1 = 293719.0;
    double v2 = -1;  // underflow
    double v3 = (double)ULONG_MAX + (double)1;  // overflow
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUINTEGER expected_v1 = static_cast< SQLUINTEGER >(
        strtoul(std::to_string(v1).c_str(), NULL, 10));
    EXPECT_EQ((SQLLEN)sizeof(SQLUINTEGER), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 3, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_SSHORT) {
    double v1 = SHRT_MIN;
    double v2 = SHRT_MAX;
    double v3 = -3.2769E4;
    double v4 = 3.2768E4;
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v1 = static_cast< SQLSMALLINT >(
        strtol(std::to_string(v1).c_str(), NULL, 10));
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v2 = static_cast< SQLSMALLINT >(
        strtol(std::to_string(v2).c_str(), NULL, 10));
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_SHORT) {
    double v1 = SHRT_MIN;
    double v2 = SHRT_MAX;
    double v3 = -DBL_MAX;
    double v4 = DBL_MAX;
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v1 = static_cast< SQLSMALLINT >(
        strtol(std::to_string(v1).c_str(), NULL, 10));
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v2 = static_cast< SQLSMALLINT >(
        strtol(std::to_string(v2).c_str(), NULL, 10));
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_USHORT) {
    double v1 = 0;
    double v2 = -1.0;  // underflow
    double v3 = 6.5536E4;  // overflow
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUSMALLINT expected_v1 = static_cast< SQLUSMALLINT >(
        strtol(std::to_string(v1).c_str(), NULL, 10));
    EXPECT_EQ((SQLLEN)sizeof(SQLUSMALLINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 3, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_SBIGINT) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    double v3 = -DBL_MAX;  // underflow
    double v4 = DBL_MAX; // overflow
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLBIGINT expected_v1 = static_cast< SQLBIGINT >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLBIGINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLBIGINT expected_v2 = static_cast< SQLBIGINT >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLBIGINT), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_UBIGINT) {
    double v1 = 2.93719E5;
    double v2 = -1.0;  // underflow
    double v3 = DBL_MAX;  //overflow
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUBIGINT expected_v1 = static_cast< SQLUBIGINT >(
        strtoull(std::to_string(v1).c_str(), NULL, 10));
    EXPECT_EQ((SQLLEN)sizeof(SQLUBIGINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 3, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_CHAR) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLCHAR data2[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_v1 = std::to_string(v1);
    ASSERT_EQ((SQLLEN)expected_v1.size(), indicator);
    EXPECT_STREQ(expected_v1.c_str(), (char*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data2, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_v2 = std::to_string(v2);
    ASSERT_EQ((SQLLEN)expected_v2.size(), indicator);
    EXPECT_STREQ(expected_v2.c_str(), (char*)data2);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_DOUBLE) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLDOUBLE data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_DOUBLE, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLDOUBLE), indicator);
    EXPECT_EQ(v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_DOUBLE, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLDOUBLE), indicator);
    EXPECT_EQ(v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_FLOAT) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    double v3 = -DBL_MAX;  // underflow
    double v4 = DBL_MAX;  // overflow
    test_string columns = CREATE_STRING("DOUBLE\'") + convert_to_test_string(v1) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', DOUBLE\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLREAL data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_FLOAT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLREAL expected_v1 = static_cast< SQLREAL >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLREAL), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_FLOAT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLREAL expected_v2 = static_cast< SQLREAL >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLREAL), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_FLOAT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_FLOAT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BIGINT_TO_SQL_C_BIT) {
    long long v1 = 0;
    long long v2 = 1;
    long long v3 = -2147483649ll;  // underflow
    long long v4 = 2147483649ll;   // overflow
    test_string columns = CREATE_STRING("BIGINT\'") + convert_to_test_string(v1) + CREATE_STRING("\', BIGINT\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\', BIGINT\'")
                           + convert_to_test_string(v3) + CREATE_STRING("\', BIGINT\'")
                           + convert_to_test_string(v4) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v1 = static_cast< SQLCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v2 = static_cast< SQLCHAR >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 4, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BIGINT_TO_SQL_C_SBIGINT) {
    long long  v1 = -2147483649ll;
    long long  v2 = 2147483649ll;
    test_string columns = CREATE_STRING("BIGINT\'") + convert_to_test_string(v1) + CREATE_STRING("\', BIGINT\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLBIGINT), indicator);
    EXPECT_EQ(v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLBIGINT), indicator);
    EXPECT_EQ(v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BIGINT_TO_SQL_C_UBIGINT) {
    long long v1 = 2147483649ll;
    long long v2 = -1ll;  // underflow
    test_string columns = CREATE_STRING("BIGINT\'") + convert_to_test_string(v1) + CREATE_STRING("\', BIGINT\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUBIGINT expected_v1 = static_cast< SQLUBIGINT > (v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLUBIGINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BIGINT_TO_SQL_C_CHAR) {
    auto v1 = 2147483649ll;
    auto v2 = 2147483649ll;
    v2 *= -1;
    test_string columns = CREATE_STRING("BIGINT\'") + convert_to_test_string(v1) + CREATE_STRING("\', BIGINT\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_v1 = std::to_string(v1);
    ASSERT_EQ((SQLLEN)expected_v1.size(), indicator);
    EXPECT_STREQ(expected_v1.c_str(), (char*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_v2 = std::to_string(v2);
    ASSERT_EQ((SQLLEN)expected_v2.size(), indicator);
    EXPECT_STREQ(expected_v2.c_str(), (char*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BOOLEAN_TO_SQL_BIT) {
    test_string columns = CREATE_STRING("true, false");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    bool data = false;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_TRUE(data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_FALSE(data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BOOLEAN_TO_SQL_C_SLONG) {
    test_string columns = CREATE_STRING("true, false");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_TRUE(data != 0);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_TRUE(data == 0);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BOOLEAN_TO_SQL_C_ULONG) {
    test_string columns = CREATE_STRING("true, false");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLUINTEGER), indicator);
    EXPECT_TRUE(data != 0);
    ret = SQLGetData(m_hstmt, 2, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLUINTEGER), indicator);
    EXPECT_TRUE(data == 0);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BOOLEAN_TO_SQL_C_CHAR) {
    test_string columns = CREATE_STRING("true, false");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ASSERT_EQ(1, indicator);
    EXPECT_EQ('1', data[0]);
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ASSERT_EQ(1, indicator);
    EXPECT_EQ('0', data[0]);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESERIES_TO_SQL_C_CHAR_ARRAY) {
    SQLRETURN ret = SQL_ERROR;
    test_string statement =
        CREATE_STRING("WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 ")
        CREATE_STRING("14:18:30.123456789' AS binned_timestamp, ARRAY[1,2,3] ")
        CREATE_STRING("AS data FROM ODBCTest.IoT LIMIT ")
        CREATE_STRING("1), interpolated_timeseries AS(SELECT ")
        CREATE_STRING("CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) ")
        CREATE_STRING("SELECT *FROM interpolated_timeseries");
    ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)statement.c_str(),
                        (SQLINTEGER)statement.length());
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    ret = SQLFetch(m_hstmt);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("[{time: 2021-03-05 14:18:30.123456789, value: [1, 2, 3]}]");
    CompareData(expected, indicator, data, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESERIES_TO_SQL_C_WCHAR_ARRAY) {
    SQLRETURN ret = SQL_ERROR;
    test_string statement =
        CREATE_STRING("WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 ")
        CREATE_STRING("14:18:30.123456789' AS binned_timestamp, ARRAY[1,2,3] ")
        CREATE_STRING("AS data FROM ODBCTest.IoT LIMIT ")
        CREATE_STRING("1), interpolated_timeseries AS(SELECT ")
        CREATE_STRING("CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) ")
        CREATE_STRING("SELECT *FROM interpolated_timeseries");
    ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)statement.c_str(),
                        (SQLINTEGER)statement.length());
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    ret = SQLFetch(m_hstmt);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLTCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("[{time: 2021-03-05 14:18:30.123456789, value: [1, 2, 3]}]");
    CompareData(expected, indicator, data, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESERIES_TO_SQL_C_CHAR_ROW) {
    SQLRETURN ret = SQL_ERROR;
    test_string statement =
        CREATE_STRING("WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 ")
        CREATE_STRING("14:18:30.123456789' AS binned_timestamp, CAST(ROW(9.9, 19) AS ")
        CREATE_STRING("ROW(sum DOUBLE, count INTEGER)) AS data FROM ODBCTest.IoT LIMIT ")
        CREATE_STRING("1), interpolated_timeseries AS(SELECT ")
        CREATE_STRING("CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) ")
        CREATE_STRING("SELECT *FROM interpolated_timeseries");
    ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)statement.c_str(), (SQLINTEGER)statement.length());
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    ret = SQLFetch(m_hstmt);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("[{time: 2021-03-05 14:18:30.123456789, value: (9.9, 19)}]");
    CompareData(expected, indicator, data, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESERIES_TO_SQL_C_WCHAR_ROW) {
    SQLRETURN ret = SQL_ERROR;
    test_string statement =
        CREATE_STRING("WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 ")
        CREATE_STRING("14:18:30.123456789' AS binned_timestamp, CAST(ROW(9.9, 19) AS ")
        CREATE_STRING("ROW(sum DOUBLE, count INTEGER)) AS data FROM ODBCTest.IoT LIMIT ")
        CREATE_STRING("1), interpolated_timeseries AS(SELECT ")
        CREATE_STRING("CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) ")
        CREATE_STRING("SELECT *FROM interpolated_timeseries");
    ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)statement.c_str(),
                        (SQLINTEGER)statement.length());
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    ret = SQLFetch(m_hstmt);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLTCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("[{time: 2021-03-05 14:18:30.123456789, value: (9.9, 19)}]");
    CompareData(expected, indicator, data, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESERIES_TO_SQL_C_CHAR_ARRAY_ROW_COMBINATION) {
    SQLRETURN ret = SQL_ERROR;
    test_string statement =
        CREATE_STRING("WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 ")
        CREATE_STRING("14:18:30.123456789' AS binned_timestamp, ROW(null, ARRAY[ARRAY[ROW(12345, ARRAY[1, 2, 3])]]) ")
        CREATE_STRING("AS data FROM ODBCTest.IoT LIMIT ")
        CREATE_STRING("1), interpolated_timeseries AS(SELECT ")
        CREATE_STRING("CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) ")
        CREATE_STRING("SELECT *FROM interpolated_timeseries");
    ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)statement.c_str(),
                        (SQLINTEGER)statement.length());
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    ret = SQLFetch(m_hstmt);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("[{time: 2021-03-05 14:18:30.123456789, value: (null, [[(12345, [1, 2, 3])]])}]");
    CompareData(expected, indicator, data, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESERIES_TO_SQL_C_WCHAR_ARRAY_ROW_COMBINATION) {
    SQLRETURN ret = SQL_ERROR;
    test_string statement =
        CREATE_STRING("WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 ")
        CREATE_STRING("14:18:30.123456789' AS binned_timestamp, ROW(null, ARRAY[ARRAY[ROW(12345, ARRAY[1, 2, 3])]]) ")
        CREATE_STRING("AS data FROM ODBCTest.IoT LIMIT ")
        CREATE_STRING("1), interpolated_timeseries AS(SELECT ")
        CREATE_STRING("CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) ")
        CREATE_STRING("SELECT *FROM interpolated_timeseries");
    ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)statement.c_str(),
                        (SQLINTEGER)statement.length());
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    ret = SQLFetch(m_hstmt);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLWCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("[{time: 2021-03-05 14:18:30.123456789, value: (null, [[(12345, [1, 2, 3])]])}]");
    CompareData(expected, indicator, data, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}


TEST_F(TestSQLGetData, ARRAY_TO_SQL_C_CHAR) {
    test_string columns = CREATE_STRING("ARRAY[ARRAY[ARRAY[ARRAY[1.1, 2.3], ARRAY[1.1, 2.3]]], ARRAY[ARRAY[ARRAY[1.1, 2.3], ARRAY[1.1, 2.3]]]], ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[1, 2, 3]]]]]]]]]]]], ARRAY[]");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLCHAR data2[1024] = {0};
    SQLCHAR data3[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("[[[[1.1, 2.3], [1.1, 2.3]]], [[[1.1, 2.3], [1.1, 2.3]]]]");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data2, 1024, &indicator);
    expected = CREATE_STRING("[[[[[[[[[[[[1, 2, 3]]]]]]]]]]]]");
    CompareData(expected, indicator, data2, ret);
    
    ret = SQLGetData(m_hstmt, 3, SQL_C_CHAR, data3, 1024, &indicator);
    expected = CREATE_STRING("-");
    CompareData(expected, indicator, data3, ret);
    
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, ARRAY_TO_SQL_C_WCHAR) {
    test_string columns =
        CREATE_STRING("ARRAY[ARRAY[ARRAY[ARRAY[1.1, 2.3], ARRAY[1.1, 2.3]]], ")
        CREATE_STRING("ARRAY[ARRAY[ARRAY[1.1, 2.3], ARRAY[1.1, 2.3]]]], ")
        CREATE_STRING("ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[1, 2, 3]]]]]]]]]]]], ")
        CREATE_STRING("ARRAY[]");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLWCHAR data[1024] = {0};
    SQLWCHAR data2[1024] = {0};
    SQLWCHAR data3[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("[[[[1.1, 2.3], [1.1, 2.3]]], [[[1.1, 2.3], [1.1, 2.3]]]]");
    CompareData(expected, indicator, data, ret);

    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data2, 1024, &indicator);
    expected = CREATE_STRING("[[[[[[[[[[[[1, 2, 3]]]]]]]]]]]]");
    CompareData(expected, indicator, data2, ret);
    
    ret = SQLGetData(m_hstmt, 3, SQL_C_WCHAR, data3, 1024, &indicator);
    expected = CREATE_STRING("-");
    CompareData(expected, indicator, data3, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, ROW_TO_SQL_C_CHAR) {
    test_string columns =
        CREATE_STRING("ROW(ROW(ROW(INTEGER '03', BIGINT '10', true), ")
        CREATE_STRING("ARRAY[ARRAY[1,2],ARRAY[1.1,2.2]])), ROW(true)");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLCHAR data2[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected;
    expected = "(((3, 10, true), [[1.0, 2.0], [1.1, 2.2]]))";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data2, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "(true)";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data2);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, ROW_TO_SQL_C_WCHAR) {
    test_string columns =
        CREATE_STRING("ROW(ROW(ROW(INTEGER '03', BIGINT '10', true), ")
        CREATE_STRING("ARRAY[ARRAY[1,2],ARRAY[1.1,2.2]])), ROW(true)");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLWCHAR data[1024] = {0};
    SQLWCHAR data2[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("(((3, 10, true), [[1.0, 2.0], [1.1, 2.2]]))");
    CompareData(expected, indicator, data, ret);

    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data2, 1024, &indicator);
    expected = CREATE_STRING("(true)");
    CompareData(expected, indicator, data2, ret);

    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, NULL_TO_SQL_C_CHAR) {
    test_string columns =
        CREATE_STRING("null, NULL");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected;
    expected = "";
    ASSERT_EQ(SQL_NULL_DATA, indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "";
    ASSERT_EQ(SQL_NULL_DATA, indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, NULL_TO_SQL_C_WCHAR) {
    test_string columns = CREATE_STRING("null, NULL");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLWCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected = "";
    ASSERT_EQ(SQL_NULL_DATA, indicator);
    EXPECT_EQ(expected, wchar_to_string(data));
    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ASSERT_EQ(SQL_NULL_DATA, indicator);
    EXPECT_EQ(expected, wchar_to_string(data));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, ARRAY_ROW_NULL_TO_SQL_C_CHAR) {
    test_string columns = CREATE_STRING("Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL])");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLCHAR data2[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected = "[(null), (null)]";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data2, 1024, &indicator);
    expected = "([null], [null])";
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data2);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, ARRAY_ROW_NULL_TO_SQL_C_WCHAR) {
    test_string columns =
        CREATE_STRING("Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL])");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLWCHAR data[1024] = {0};
    SQLWCHAR data2[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("[(null), (null)]");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data2, 1024, &indicator);
    expected = CREATE_STRING("([null], [null])");
    CompareData(expected, indicator, data2, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESTAMP_TO_SQL_C_CHAR) {
    test_string columns =
        CREATE_STRING("TIMESTAMP \'2021-01-02 18:01:13.000000000\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 18:01:13.123456789\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 18:01:13.12345\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 18:01:13\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 18:01\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("2021-01-02 18:01:13.000000000");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data, 1024, &indicator);
    expected = CREATE_STRING("2021-11-20 18:01:13.123456789");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 3, SQL_C_CHAR, data, 1024, &indicator);
    expected = CREATE_STRING( "2021-11-20 18:01:13.123450000");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 4, SQL_C_CHAR, data, 1024, &indicator);
    expected = CREATE_STRING("2021-11-20 18:01:13.000000000");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 5, SQL_C_CHAR, data, 1024, &indicator);
    expected = CREATE_STRING("2021-11-20 18:01:00.000000000");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 6, SQL_C_CHAR, data, 1024, &indicator);
    expected = CREATE_STRING("2021-11-20 00:00:00.000000000");
    CompareData(expected, indicator, data, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESTAMP_TO_SQL_C_WCHAR) {
    test_string columns =
        CREATE_STRING("TIMESTAMP \'2021-01-02 18:01:13.000000000\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 18:01:13.123456789\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 18:01:13.12345\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 18:01:13\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 18:01\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLWCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("2021-01-02 18:01:13.000000000");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data, 1024, &indicator);
    expected = CREATE_STRING("2021-11-20 18:01:13.123456789");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 3, SQL_C_WCHAR, data, 1024, &indicator);
    expected = CREATE_STRING("2021-11-20 18:01:13.123450000");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 4, SQL_C_WCHAR, data, 1024, &indicator);
    expected = CREATE_STRING("2021-11-20 18:01:13.000000000");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 5, SQL_C_WCHAR, data, 1024, &indicator);
    expected = CREATE_STRING("2021-11-20 18:01:00.000000000");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 6, SQL_C_WCHAR, data, 1024, &indicator);
    expected = CREATE_STRING("2021-11-20 00:00:00.000000000");
    CompareData(expected, indicator, data, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESTAMP_TO_SQL_C_TYPE_TIMESTAMP) {
    test_string columns =
        CREATE_STRING("TIMESTAMP \'2021-01-02 18:01:13.000000000\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 06:39:45.123456789\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 06:39:45.12345\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 06:39:45\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 06:39\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20\'");

    std::vector< std::pair< TIMESTAMP_STRUCT, SQLTCHAR* > > expected;
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 1, 2, 18, 1, 13, 0}, nullptr));
    expected.push_back(std::make_pair(
        TIMESTAMP_STRUCT{2021, 11, 20, 6, 39, 45, 123456789}, nullptr));
    expected.push_back(std::make_pair(
        TIMESTAMP_STRUCT{2021, 11, 20, 6, 39, 45, 123450000}, nullptr));
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 11, 20, 6, 39, 45, 0}, nullptr));
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 11, 20, 6, 39, 0, 0}, nullptr));
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 11, 20, 0, 0, 0, 0}, nullptr));

    TestConvertingToTimestamp(m_hstmt, columns, expected);
}

TEST_F(TestSQLGetData, TIMESTAMP_TO_SQL_C_TYPE_DATE) {
    test_string columns =
        CREATE_STRING("TIMESTAMP \'2021-01-02 18:01:13.000000000\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 06:39:45.123456789\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 06:39:45.12345\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 06:39:45\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 06:39\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20\'");

    std::vector< std::pair< DATE_STRUCT, SQLTCHAR* > > expected;
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 1, 2},
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 20},
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 20},
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 20},
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 20},
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 20}, nullptr));

    TestConvertingToDate(m_hstmt, columns, expected);
}

TEST_F(TestSQLGetData, TIMESTAMP_TO_SQL_C_TYPE_TIME) {
    test_string columns = 
        CREATE_STRING("TIMESTAMP \'2021-01-02 18:01:13.000000000\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 06:39:45.123456789\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 06:39:45.12345\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 06:39:45\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20 06:39\',")
        CREATE_STRING("TIMESTAMP \'2021-11-20\'");

    std::vector< std::pair< TIME_STRUCT, SQLTCHAR* > > expected;
    expected.push_back(std::make_pair(TIME_STRUCT{18, 1, 13}, nullptr));
    expected.push_back(
        std::make_pair(TIME_STRUCT{6, 39, 45}, SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(
        std::make_pair(TIME_STRUCT{6, 39, 45}, SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(TIME_STRUCT{6, 39, 45}, nullptr));
    expected.push_back(std::make_pair(TIME_STRUCT{6, 39, 0}, nullptr));
    expected.push_back(std::make_pair(TIME_STRUCT{0, 0, 0}, nullptr));

    TestConvertingToTime(m_hstmt, columns, expected);
}

TEST_F(TestSQLGetData, DATE_TO_SQL_C_CHAR) {
    test_string columns = CREATE_STRING("DATE \'2021-01-02\', DATE \'2021-11-20\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;

    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("2021-01-02");
    CompareData(expected, indicator, data, ret);

    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data, 1024, &indicator);
    expected = CREATE_STRING("2021-11-20");
    CompareData(expected, indicator, data, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DATE_TO_SQL_C_WCHAR) {
    test_string columns = CREATE_STRING("DATE \'2021-01-02\', DATE \'2021-11-20\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;

    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("2021-01-02");
    CompareData(expected, indicator, data, ret);

    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data, 1024, &indicator);
    expected = CREATE_STRING("2021-11-20");
    CompareData(expected, indicator, data, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DATE_TO_SQL_C_TYPE_TIMESTAMP) {
    test_string columns = CREATE_STRING("DATE \'2021-01-02\', DATE \'2021-11-20\'");

    std::vector< std::pair< TIMESTAMP_STRUCT, SQLTCHAR* > > expected;
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 1, 2, 0, 0, 0, 0}, nullptr));
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 11, 20, 0, 0, 0, 0}, nullptr));

    TestConvertingToTimestamp(m_hstmt, columns, expected);
}

TEST_F(TestSQLGetData, DATE_TO_SQL_C_TYPE_DATE) {
    test_string columns = CREATE_STRING("DATE \'2021-01-02\', DATE \'2021-11-20\'");

    std::vector< std::pair< DATE_STRUCT, SQLTCHAR* > > expected;
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 1, 2}, nullptr));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 20}, nullptr));

    TestConvertingToDate(m_hstmt, columns, expected);
}

TEST_F(TestSQLGetData, DATE_TO_SQL_C_TYPE_TIME) {
    test_string columns = CREATE_STRING("DATE \'2021-01-02\', DATE \'2021-11-20\'");

    std::vector< std::pair< TIME_STRUCT, SQLTCHAR* > > expected;
    expected.push_back(
        std::make_pair(TIME_STRUCT{0, 0, 0}, SQLSTATE_RESTRICTED_DATA_TYPE_ERROR));
    expected.push_back(
        std::make_pair(TIME_STRUCT{0, 0, 0}, SQLSTATE_RESTRICTED_DATA_TYPE_ERROR));

    TestConvertingToTime(m_hstmt, columns, expected);
}

TEST_F(TestSQLGetData, TIME_TO_SQL_C_CHAR) {
    test_string columns =
        CREATE_STRING("TIME \'18:01:13.000000000\',")
        CREATE_STRING("TIME \'06:39:45.123456789\',")
        CREATE_STRING("TIME \'06:39:45.12345\',")
        CREATE_STRING("TIME \'06:39:45\',")
        CREATE_STRING("TIME \'06:39\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("18:01:13.000000000");
    CompareData(expected, indicator, data, ret);

    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data, 1024, &indicator);
    expected = CREATE_STRING("06:39:45.123456789");
    CompareData(expected, indicator, data, ret);

    ret = SQLGetData(m_hstmt, 3, SQL_C_CHAR, data, 1024, &indicator);
    expected = CREATE_STRING("06:39:45.123450000");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 4, SQL_C_CHAR, data, 1024, &indicator);
    expected = CREATE_STRING("06:39:45.000000000");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 5, SQL_C_CHAR, data, 1024, &indicator);
    expected = CREATE_STRING("06:39:00.000000000");
    CompareData(expected, indicator, data, ret);
    
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIME_TO_SQL_C_WCHAR) {
    test_string columns =
        CREATE_STRING("TIME \'18:01:13.000000000\',")
        CREATE_STRING("TIME \'06:39:45.123456789\',")
        CREATE_STRING("TIME \'06:39:45.12345\',")
        CREATE_STRING("TIME \'06:39:45\',")
        CREATE_STRING("TIME \'06:39\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("18:01:13.000000000");
    CompareData(expected, indicator, data, ret);

    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data, 1024, &indicator);
    expected = CREATE_STRING("06:39:45.123456789");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 3, SQL_C_WCHAR, data, 1024, &indicator);
    expected = CREATE_STRING("06:39:45.123450000");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 4, SQL_C_WCHAR, data, 1024, &indicator);
    expected = CREATE_STRING("06:39:45.000000000");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 5, SQL_C_WCHAR, data, 1024, &indicator);
    expected = CREATE_STRING("06:39:00.000000000");
    CompareData(expected, indicator, data, ret);
    
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIME_TO_SQL_C_TYPE_TIMESTAMP) {
    test_string columns =
        CREATE_STRING("TIME \'18:01:13.000000000\',")
        CREATE_STRING("TIME \'06:39:45.123456789\',")
        CREATE_STRING("TIME \'06:39:45.12345\',")
        CREATE_STRING("TIME \'06:39:45\',")
        CREATE_STRING("TIME \'06:39\'");

    time_t rawtime;
    struct tm* now;
    time(&rawtime);
    now = localtime(&rawtime);
    SQLSMALLINT year = static_cast< SQLSMALLINT >(now->tm_year + 1900);
    SQLUSMALLINT month = static_cast< SQLUSMALLINT >(now->tm_mon + 1);
    SQLUSMALLINT day = static_cast< SQLUSMALLINT >(now->tm_mday);

    std::vector< std::pair< TIMESTAMP_STRUCT, SQLTCHAR* > > expected;
    expected.push_back(std::make_pair(
        TIMESTAMP_STRUCT{year, month, day, 18, 1, 13, 0}, nullptr));
    expected.push_back(std::make_pair(
        TIMESTAMP_STRUCT{year, month, day, 6, 39, 45, 123456789}, nullptr));
    expected.push_back(std::make_pair(
        TIMESTAMP_STRUCT{year, month, day, 6, 39, 45, 123450000}, nullptr));
    expected.push_back(std::make_pair(
        TIMESTAMP_STRUCT{year, month, day, 6, 39, 45, 0}, nullptr));
    expected.push_back(std::make_pair(
        TIMESTAMP_STRUCT{year, month, day, 6, 39, 0, 0}, nullptr));

    TestConvertingToTimestamp(m_hstmt, columns, expected);
}

TEST_F(TestSQLGetData, TIME_TO_SQL_C_TYPE_DATE) {
    test_string columns =
        CREATE_STRING("TIME \'18:01:13.524000000\', TIME \'06:39:45.123456789\'");

    std::vector< std::pair< DATE_STRUCT, SQLTCHAR* > > expected;
    expected.push_back(
        std::make_pair(DATE_STRUCT{0, 0, 0}, SQLSTATE_RESTRICTED_DATA_TYPE_ERROR));
    expected.push_back(
        std::make_pair(DATE_STRUCT{0, 0, 0}, SQLSTATE_RESTRICTED_DATA_TYPE_ERROR));

    TestConvertingToDate(m_hstmt, columns, expected);
}

TEST_F(TestSQLGetData, TIME_TO_SQL_C_TYPE_TIME) {
    test_string columns =
        CREATE_STRING("TIME \'18:01:13.000000000\',")
        CREATE_STRING("TIME \'06:39:45.123456789\',")
        CREATE_STRING("TIME \'06:39:45.12345\',")
        CREATE_STRING("TIME \'06:39:45\',")
        CREATE_STRING("TIME \'06:39\'");

    std::vector< std::pair< TIME_STRUCT, SQLTCHAR* > > expected;
    expected.push_back(std::make_pair(TIME_STRUCT{18, 1, 13}, nullptr));
    expected.push_back(std::make_pair(TIME_STRUCT{6, 39, 45}, nullptr));
    expected.push_back(std::make_pair(TIME_STRUCT{6, 39, 45}, nullptr));
    expected.push_back(std::make_pair(TIME_STRUCT{6, 39, 45}, nullptr));
    expected.push_back(std::make_pair(TIME_STRUCT{6, 39, 0}, nullptr));

    TestConvertingToTime(m_hstmt, columns, expected);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_BIT) {
    int v1 = 0;
    int v2 = 1;
    double v3 = 1.5;  // truncation
    int v4 = -1;  // underflow
    int v5 = 2;  // overflow
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v3)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v3) // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v4) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v5)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'"); // not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v1 = static_cast< SQLCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v2 = static_cast< SQLCHAR >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    SQLCHAR expected_v3 = static_cast< SQLCHAR >(v3);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v3, data);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_FRACTIONAL_TRUNCATION));
    ret = SQLGetData(m_hstmt, 5, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 7, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_STINYINT) {
    int v1 = SCHAR_MIN;
    int v2 = SCHAR_MAX;
    double v3 = 1.5; // truncation
    int v4 = SCHAR_MIN - 1;  // underflow
    int v5 = SCHAR_MAX + 1;  // overflow
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v3)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v3) // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v4) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v5)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'"); // not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v1 = static_cast< SQLSCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v2 = static_cast< SQLSCHAR >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    SQLCHAR expected_v3 = static_cast< SQLSCHAR >(v3);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 5, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 7, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_TINYINT) {
    int v1 = SCHAR_MIN;
    int v2 = SCHAR_MAX;
    double v3 = 1.5;  // truncation
    int v4 = INT_MIN;  // underflow
    int v5 = INT_MAX;  // overflow
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v3)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v3) // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v4) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v5)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'"); // Not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v1 = static_cast< SQLSCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v2 = static_cast< SQLSCHAR >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    SQLCHAR expected_v3 = static_cast< SQLSCHAR >(v3);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_EQ((SQLLEN)sizeof(SQLSCHAR), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 5, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 7, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_UTINYINT) {
    int v1 = UCHAR_MAX;
    double v2 = 1.5;         // truncation
    int v3 = -1;             // underflow
    int v4 = UCHAR_MAX + 1;  // overflow
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v2) // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v3) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v4)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'"); // not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v1 = static_cast< SQLCHAR >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    SQLCHAR expected_v2 = static_cast< SQLCHAR >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_EQ((SQLLEN)sizeof(SQLCHAR), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 5, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_SLONG) {
    int v1 = -293719;
    int v2 = 741370;
    double v3 = 1.5;  // truncation
    double v4 = -9.3E18;  // underflow
    double v5 = (double)LONG_MAX + (double)1;  // overflow  
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v3)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v3) // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v4) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v5)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'"); // Not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v1 = static_cast< SQLINTEGER >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v2 = static_cast< SQLINTEGER >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    SQLINTEGER expected_v3 = static_cast< SQLINTEGER >(v3);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 5, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 7, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_LONG) {
    int v1 = -293719;
    int v2 = 741370;
    double v3 = 1.5;  // truncation
    double v4 = -DBL_MAX;  // underflow
    double v5 = DBL_MAX;  // overflow
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v3)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v3) // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v4) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v5)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'"); // Not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v1 = static_cast< SQLINTEGER >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v2 = static_cast< SQLINTEGER >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    SQLINTEGER expected_v3 = static_cast< SQLINTEGER >(v3);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 5, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 7, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_ULONG) {
    int v1 = 293719;
    double v2 = 1.5;  // truncation
    int v3 = -1;  // underflow
    double v4 = (double)ULONG_MAX + (double)1;  // overflow
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v2) // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v3) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v4)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'"); // not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUINTEGER expected_v1 = static_cast< SQLUINTEGER >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLUINTEGER), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    SQLUINTEGER expected_v2 = static_cast< SQLUINTEGER >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLUINTEGER), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_EQ((SQLLEN)sizeof(SQLUINTEGER), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 5, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_SSHORT) {
    int v1 = SHRT_MIN;
    int v2 = SHRT_MAX;
    double v3 = 1.5;  // truncation
    int v4 = SHRT_MIN - 1;  // underflow
    int v5 = SHRT_MAX + 1;  // overflow
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v3)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v3) // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v4) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v5)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'"); // not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v1 = static_cast< SQLSMALLINT >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v2 = static_cast< SQLSMALLINT >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    SQLSMALLINT expected_v3 = static_cast< SQLSMALLINT >(v3);
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 5, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 7, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_SHORT) {
    int v1 = SHRT_MIN;
    int v2 = SHRT_MAX;
    double v3 = 1.5;  // truncation
    int v4 = INT_MIN;  // underflow
    int v5 = INT_MAX;  // overflow
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v3)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v3)  // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v4) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v5)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'");  // not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v1 = static_cast< SQLSMALLINT >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v2 = static_cast< SQLSMALLINT >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    SQLSMALLINT expected_v3 = static_cast< SQLSMALLINT >(v3);
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_EQ((SQLLEN)sizeof(SQLSMALLINT), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 5, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 7, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_USHORT) {
    int v1 = USHRT_MAX;
    double v2 = 1.5;         // truncation
    int v3 = -1;             // underflow
    int v4 = USHRT_MAX + 1;  // overflow
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v2) // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v3) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v4)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'"); // not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUSMALLINT expected_v1 = static_cast< SQLUSMALLINT >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLUSMALLINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    SQLUSMALLINT expected_v2 = static_cast< SQLUSMALLINT >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLUSMALLINT), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_EQ((SQLLEN)sizeof(SQLUSMALLINT), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 5, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_SBIGINT) {
    int v1 = -293719;
    int v2 = 741370;
    double v3 = 1.5;  // truncation
    double v4 = -DBL_MAX;  // underflow
    double v5 = DBL_MAX;  // overflow
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v3)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v3)  // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v4) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v5)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'");  // not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLBIGINT expected_v1 = static_cast< SQLBIGINT >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLBIGINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLBIGINT expected_v2 = static_cast< SQLBIGINT >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLBIGINT), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    SQLBIGINT expected_v3 = static_cast< SQLBIGINT >(v3);
    EXPECT_EQ((SQLLEN)sizeof(SQLBIGINT), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_EQ((SQLLEN)sizeof(SQLBIGINT), indicator);
    EXPECT_EQ(expected_v3, data);
    ret = SQLGetData(m_hstmt, 5, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 7, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_UBIGINT) {
    int v1 = 293719;
    double v2 = 1.5;
    double v3 = -1.0;  // underflow
    double v4 = DBL_MAX;  //overflow
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v2) // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v3) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v4)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'"); // not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUBIGINT expected_v1 = static_cast< SQLUBIGINT >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLUBIGINT), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    SQLUBIGINT expected_v2 = static_cast< SQLUBIGINT >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLUBIGINT), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_EQ((SQLLEN)sizeof(SQLUBIGINT), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 5, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_DOUBLE) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v2)  // leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'")
        + CREATE_STRING("9179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368") // overflow
        + CREATE_STRING("\', VARCHAR\'")
        + CREATE_STRING("-9179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368") // underflow
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'"); // not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLDOUBLE data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_DOUBLE, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLDOUBLE), indicator);
    EXPECT_EQ(v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_DOUBLE, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLDOUBLE), indicator);
    EXPECT_EQ(v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_DOUBLE, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLDOUBLE), indicator);
    EXPECT_EQ(v2, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_DOUBLE, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 5, SQL_C_DOUBLE, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_DOUBLE, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_FLOAT) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    double v3 = -DBL_MAX;  // underflow
    double v4 = DBL_MAX;   // overflow
    test_string columns =
        CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1)
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v2)
        + CREATE_STRING("\', VARCHAR\'   ") + convert_to_test_string(v2) // truncation with leading and trailing spaces
        + CREATE_STRING("  \', VARCHAR\'") + convert_to_test_string(v3) 
        + CREATE_STRING("\', VARCHAR\'") + convert_to_test_string(v4)
        + CREATE_STRING("\', VARCHAR\'") + CREATE_STRING("1.a") + CREATE_STRING("\'"); // not a numeric literal
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLREAL data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_FLOAT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLREAL expected_v1 = static_cast< SQLREAL >(v1);
    EXPECT_EQ((SQLLEN)sizeof(SQLREAL), indicator);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_FLOAT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLREAL expected_v2 = static_cast< SQLREAL >(v2);
    EXPECT_EQ((SQLLEN)sizeof(SQLREAL), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_FLOAT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQLREAL), indicator);
    EXPECT_EQ(expected_v2, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_FLOAT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 5, SQL_C_FLOAT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
    ret = SQLGetData(m_hstmt, 6, SQL_C_FLOAT, &data, 0, &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_CHAR) {
    int v1 = -293719;
    int v2 = 741370;
    test_string columns = CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1) + CREATE_STRING("\', VARCHAR\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLCHAR data2[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_v1 = std::to_string(v1);
    ASSERT_EQ((SQLLEN)expected_v1.size(), indicator);
    EXPECT_STREQ(expected_v1.c_str(), (char*)data);
    SQLLEN expected_size = std::to_string(v2).size();
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data2, expected_size,
                     &indicator);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_DATA_RIGHT_TRUNCATED));
    ASSERT_EQ(expected_size, indicator);
    char expected_v2[1024] = {0};
    strncpy(expected_v2, std::to_string(v2).c_str(),
            static_cast< size_t >(expected_size - 1));
    EXPECT_STREQ(expected_v2, (char*)data2);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_WCHAR) {
    int v1 = -293719;
    int v2 = 741370;
    test_string columns = CREATE_STRING("VARCHAR\'") + convert_to_test_string(v1) + CREATE_STRING("\', VARCHAR\'")
                           + convert_to_test_string(v2) + CREATE_STRING("\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLWCHAR data[1024] = {0};
    SQLWCHAR data2[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    test_string expected = convert_to_test_string(v1);
    CompareData(expected, indicator, data, ret);
    
    SQLLEN expected_size = convert_to_test_string(v2).size();
#ifdef __APPLE__
    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data2, 4 * (expected_size),
                     &indicator);
#else
    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data2, 2 * (expected_size),
                     &indicator);
#endif
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_DATA_RIGHT_TRUNCATED));
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected_size), indicator);
#else
    ASSERT_EQ((int)(2 * expected_size), indicator);
#endif
    std::string expected_v2 = std::to_string(v2);
    EXPECT_EQ(expected_v2.substr(0, expected_v2.size() - 1), tchar_to_string(data2));

    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_TYPE_TIMESTAMP) {
    test_string columns =
        CREATE_STRING("VARCHAR \'2021-01-02 18:01:13.000000000\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45.123456789\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45.12345\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06\',")
        CREATE_STRING("VARCHAR \'2021-11-20\',")
        CREATE_STRING("VARCHAR \'2021-11\',")
        CREATE_STRING("VARCHAR \'06:39:45.123456789\',")
        CREATE_STRING("VARCHAR \'06:39:45\',")
        CREATE_STRING("VARCHAR \'06:39\',")
        CREATE_STRING("VARCHAR \'   2021-01-02 18:01:13.000000000   \',")
        // invalid
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45.1234567890\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45a\'");

    time_t rawtime;
    struct tm* now;
    time(&rawtime);
    now = localtime(&rawtime);
    SQLSMALLINT year = static_cast< SQLSMALLINT >(now->tm_year + 1900);
    SQLUSMALLINT month = static_cast< SQLUSMALLINT >(now->tm_mon + 1);
    SQLUSMALLINT day = static_cast< SQLUSMALLINT >(now->tm_mday);

    std::vector< std::pair< TIMESTAMP_STRUCT, SQLTCHAR* > > expected;
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 1, 2, 18, 1, 13, 0}, nullptr));
    expected.push_back(std::make_pair(
        TIMESTAMP_STRUCT{2021, 11, 20, 6, 39, 45, 123456789}, nullptr));
    expected.push_back(std::make_pair(
        TIMESTAMP_STRUCT{2021, 11, 20, 6, 39, 45, 123450000}, nullptr));
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 11, 20, 6, 39, 45, 0}, nullptr));
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 11, 20, 6, 39, 0, 0}, nullptr));
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 11, 20, 6, 0, 0, 0}, nullptr));
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 11, 20, 0, 0, 0, 0}, nullptr));
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 11, 1, 0, 0, 0, 0}, nullptr));
    expected.push_back(std::make_pair(
        TIMESTAMP_STRUCT{year, month, day, 6, 39, 45, 123456789}, nullptr));
    expected.push_back(std::make_pair(
        TIMESTAMP_STRUCT{year, month, day, 6, 39, 45, 0}, nullptr));
    expected.push_back(std::make_pair(
        TIMESTAMP_STRUCT{year, month, day, 6, 39, 0, 0}, nullptr));
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{2021, 1, 2, 18, 1, 13, 0}, nullptr));
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{0, 0, 0, 0, 0, 0, 0}, SQLSTATE_STRING_CONVERSION_ERROR));
    expected.push_back(
        std::make_pair(TIMESTAMP_STRUCT{0 0, 0, 0, 0, 0, 0}, SQLSTATE_STRING_CONVERSION_ERROR));

    TestConvertingToTimestamp(m_hstmt, columns, expected);
}


TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_TYPE_DATE) {
    test_string columns =
        CREATE_STRING("VARCHAR \'2021-01-02 18:01:13.000000000\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45.123456789\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45.12345\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06\',")
        CREATE_STRING("VARCHAR \'   2021-01-02 18:01:13.000000000   \',")
        CREATE_STRING("VARCHAR \'2021-11-20\',")
        CREATE_STRING("VARCHAR \'2021-11\',")
        // invalid
        CREATE_STRING("VARCHAR \'06:39:45.123456789\',")
        CREATE_STRING("VARCHAR \'06:39:45\',")
        CREATE_STRING("VARCHAR \'06:39\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45.1234567890\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45a\'");

    std::vector< std::pair< DATE_STRUCT, SQLTCHAR* > > expected;
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 1, 2},
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 20},
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 20},
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 20},
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 20},
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 20},
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 1, 2},
                                      SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 20}, nullptr));
    expected.push_back(std::make_pair(DATE_STRUCT{2021, 11, 1}, nullptr));
    expected.push_back(
        std::make_pair(DATE_STRUCT{0, 0, 0}, SQLSTATE_STRING_CONVERSION_ERROR));
    expected.push_back(
        std::make_pair(DATE_STRUCT{0, 0, 0}, SQLSTATE_STRING_CONVERSION_ERROR));
    expected.push_back(
        std::make_pair(DATE_STRUCT{0, 0, 0}, SQLSTATE_STRING_CONVERSION_ERROR));
    expected.push_back(
        std::make_pair(DATE_STRUCT{0, 0, 0}, SQLSTATE_STRING_CONVERSION_ERROR));
    expected.push_back(
        std::make_pair(DATE_STRUCT{0, 0, 0}, SQLSTATE_STRING_CONVERSION_ERROR));

    TestConvertingToDate(m_hstmt, columns, expected);
}

TEST_F(TestSQLGetData, VARCHAR_TO_SQL_C_TYPE_TIME) {
    test_string columns =
        CREATE_STRING("VARCHAR \'2021-01-02 18:01:13.000000000\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45.123456789\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45.12345\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06\',")
        CREATE_STRING("VARCHAR \'06:39:45.123456789\',")
        CREATE_STRING("VARCHAR \'06:39:45\',")
        CREATE_STRING("VARCHAR \'06:39\',")
        CREATE_STRING("VARCHAR \'   2021-01-02 18:01:13.000000000   \',")
        // invalid
        CREATE_STRING("VARCHAR \'2021-11-20\',")
        CREATE_STRING("VARCHAR \'2021-11\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45.1234567890\',")
        CREATE_STRING("VARCHAR \'2021-11-20 06:39:45a\'");

    std::vector< std::pair< TIME_STRUCT, SQLWCHAR* > > expected;
    expected.push_back(std::make_pair(TIME_STRUCT{18, 1, 13}, nullptr));
    expected.push_back(
        std::make_pair(TIME_STRUCT{6, 39, 45}, SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(
        std::make_pair(TIME_STRUCT{6, 39, 45}, SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(TIME_STRUCT{6, 39, 45}, nullptr));
    expected.push_back(std::make_pair(TIME_STRUCT{6, 39, 0}, nullptr));
    expected.push_back(std::make_pair(TIME_STRUCT{6, 0, 0}, nullptr));
    expected.push_back(
        std::make_pair(TIME_STRUCT{6, 39, 45}, SQLSTATE_FRACTIONAL_TRUNCATION));
    expected.push_back(std::make_pair(TIME_STRUCT{6, 39, 45}, nullptr));
    expected.push_back(std::make_pair(TIME_STRUCT{6, 39, 0}, nullptr));
    expected.push_back(std::make_pair(TIME_STRUCT{18, 1, 13}, nullptr));
    expected.push_back(
        std::make_pair(TIME_STRUCT{0, 0, 0}, SQLSTATE_STRING_CONVERSION_ERROR));
    expected.push_back(
        std::make_pair(TIME_STRUCT{0, 0, 0}, SQLSTATE_STRING_CONVERSION_ERROR));
    expected.push_back(
        std::make_pair(TIME_STRUCT{0, 0, 0}, SQLSTATE_STRING_CONVERSION_ERROR));
    expected.push_back(
        std::make_pair(TIME_STRUCT{0, 0, 0}, SQLSTATE_STRING_CONVERSION_ERROR));

    TestConvertingToTime(m_hstmt, columns, expected);
}

TEST_F(TestSQLGetData, VARCHAR_TO_INTERVAL_YEAR_TO_MONTH) {
    test_string columns =
        CREATE_STRING("VARCHAR \'1-0',")
        CREATE_STRING("VARCHAR \'0-1\',")
        CREATE_STRING("VARCHAR \'-1-0\',")
        CREATE_STRING("VARCHAR \'-0-1\',")
        CREATE_STRING("VARCHAR \'0-0\',")
        CREATE_STRING("VARCHAR \'a0-0\',")
        CREATE_STRING("VARCHAR \'   0-0   \'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQL_INTERVAL_STRUCT data;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_INTERVAL_YEAR_TO_MONTH, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    SQL_INTERVAL_STRUCT is1 = ConstructIntervalStruct(
        SQL_IS_YEAR_TO_MONTH, SQL_FALSE, 1, 0, 0, 0, 0, 0, 0);
    CompareIntervalStruct(is1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_INTERVAL_YEAR_TO_MONTH, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    SQL_INTERVAL_STRUCT is2 = ConstructIntervalStruct(
        SQL_IS_YEAR_TO_MONTH, SQL_FALSE, 0, 1, 0, 0, 0, 0, 0);
    CompareIntervalStruct(is2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_INTERVAL_YEAR_TO_MONTH, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    SQL_INTERVAL_STRUCT is3 = ConstructIntervalStruct(
        SQL_IS_YEAR_TO_MONTH, SQL_TRUE, 1, 0, 0, 0, 0, 0, 0);
    CompareIntervalStruct(is3, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_INTERVAL_YEAR_TO_MONTH, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    SQL_INTERVAL_STRUCT is4 = ConstructIntervalStruct(
        SQL_IS_YEAR_TO_MONTH, SQL_TRUE, 0, 1, 0, 0, 0, 0, 0);
    CompareIntervalStruct(is4, data);
    ret = SQLGetData(m_hstmt, 5, SQL_C_INTERVAL_YEAR_TO_MONTH, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    SQL_INTERVAL_STRUCT is5 = ConstructIntervalStruct(
        SQL_IS_YEAR_TO_MONTH, SQL_FALSE, 0, 0, 0, 0, 0, 0, 0);
    CompareIntervalStruct(is5, data);
    ret = SQLGetData(m_hstmt, 6, SQL_C_INTERVAL_YEAR_TO_MONTH, &data, 0,
                     &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    ret = SQLGetData(m_hstmt, 7, SQL_C_INTERVAL_YEAR_TO_MONTH, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    CompareIntervalStruct(is5, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, VARCHAR_TO_INTERVAL_DAY_TO_SECOND) {
    test_string columns =
        CREATE_STRING("VARCHAR \'1 00:00:00.000000000\',")
        CREATE_STRING("VARCHAR \'0 01:00:00.000000000\',")
        CREATE_STRING("VARCHAR \'0 00:01:00.000000000\',")
        CREATE_STRING("VARCHAR \'0 00:00:01.000000000\',")
        CREATE_STRING("VARCHAR \'0 00:00:00.001000000\',")
        CREATE_STRING("VARCHAR \'0 00:00:00.000001000\',")
        CREATE_STRING("VARCHAR \'0 00:00:00.000000001\',")
        CREATE_STRING("VARCHAR \'a0 00:00:00.000000001\',")
        CREATE_STRING("VARCHAR \'   0 00:00:00.000000001   \'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQL_INTERVAL_STRUCT data;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_INTERVAL_DAY_TO_SECOND, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    SQL_INTERVAL_STRUCT is1 = ConstructIntervalStruct(
        SQL_IS_DAY_TO_SECOND, SQL_FALSE, 0, 0, 1, 0, 0, 0, 0);
    CompareIntervalStruct(is1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_INTERVAL_DAY_TO_SECOND, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    SQL_INTERVAL_STRUCT is2 = ConstructIntervalStruct(
        SQL_IS_DAY_TO_SECOND, SQL_FALSE, 0, 0, 0, 1, 0, 0, 0);
    CompareIntervalStruct(is2, data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_INTERVAL_DAY_TO_SECOND, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    SQL_INTERVAL_STRUCT is3 = ConstructIntervalStruct(
        SQL_IS_DAY_TO_SECOND, SQL_FALSE, 0, 0, 0, 0, 1, 0, 0);
    CompareIntervalStruct(is3, data);
    ret = SQLGetData(m_hstmt, 4, SQL_C_INTERVAL_DAY_TO_SECOND, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    SQL_INTERVAL_STRUCT is4 = ConstructIntervalStruct(
        SQL_IS_DAY_TO_SECOND, SQL_FALSE, 0, 0, 0, 0, 0, 1, 0);
    CompareIntervalStruct(is4, data);
    ret = SQLGetData(m_hstmt, 5, SQL_C_INTERVAL_DAY_TO_SECOND, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    SQL_INTERVAL_STRUCT is5 = ConstructIntervalStruct(
        SQL_IS_DAY_TO_SECOND, SQL_FALSE, 0, 0, 0, 0, 0, 0, 1000000);
    CompareIntervalStruct(is5, data);
    ret = SQLGetData(m_hstmt, 6, SQL_C_INTERVAL_DAY_TO_SECOND, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    SQL_INTERVAL_STRUCT is6 = ConstructIntervalStruct(
        SQL_IS_DAY_TO_SECOND, SQL_FALSE, 0, 0, 0, 0, 0, 0, 1000);
    CompareIntervalStruct(is6, data);
    ret = SQLGetData(m_hstmt, 7, SQL_C_INTERVAL_DAY_TO_SECOND, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    SQL_INTERVAL_STRUCT is7 = ConstructIntervalStruct(
        SQL_IS_DAY_TO_SECOND, SQL_FALSE, 0, 0, 0, 0, 0, 0, 1);
    CompareIntervalStruct(is7, data);
    ret = SQLGetData(m_hstmt, 8, SQL_C_INTERVAL_DAY_TO_SECOND, &data, 0,
                     &indicator);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_CONVERSION_ERROR));
    ret = SQLGetData(m_hstmt, 9, SQL_C_INTERVAL_DAY_TO_SECOND, &data, 0,
                     &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ((SQLLEN)sizeof(SQL_INTERVAL_STRUCT), indicator);
    CompareIntervalStruct(is7, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTERVAL_YEAR_TO_MONTH_TO_SQL_C_CHAR) {
    test_string columns =
        CREATE_STRING("1year,")
        CREATE_STRING("1month,")
        CREATE_STRING("-1year,")
        CREATE_STRING("-1month,")
        CREATE_STRING("0year,")
        CREATE_STRING("0month");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLCHAR data2[1024] = {0};
    SQLCHAR data3[1024] = {0};
    SQLCHAR data4[1024] = {0};
    SQLCHAR data5[1024] = {0};
    SQLCHAR data6[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected;
    expected = "1-0";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data2, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "0-1";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data2);
    ret = SQLGetData(m_hstmt, 3, SQL_C_CHAR, data3, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "-1-0";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data3);
    ret = SQLGetData(m_hstmt, 4, SQL_C_CHAR, data4, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "-0-1";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data4);
    ret = SQLGetData(m_hstmt, 5, SQL_C_CHAR, data5, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "0-0";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data5);
    ret = SQLGetData(m_hstmt, 6, SQL_C_CHAR, data6, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "0-0";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data6);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTERVAL_YEAR_TO_MONTH_TO_SQL_C_WCHAR) {
    test_string columns =
        CREATE_STRING("1year,")
        CREATE_STRING("1month,")
        CREATE_STRING("-1year,")
        CREATE_STRING("-1month,")
        CREATE_STRING("0year,")
        CREATE_STRING("0month");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLWCHAR data[1024] = {0};
    SQLWCHAR data2[1024] = {0};
    SQLWCHAR data3[1024] = {0};
    SQLWCHAR data4[1024] = {0};
    SQLWCHAR data5[1024] = {0};
    SQLWCHAR data6[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("1-0");
    CompareData(expected, indicator, data, ret);
    
    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data2, 1024, &indicator);
    expected = CREATE_STRING("0-1");
    CompareData(expected, indicator, data2, ret);

    ret = SQLGetData(m_hstmt, 3, SQL_C_WCHAR, data3, 1024, &indicator);
    expected = CREATE_STRING("-1-0");
    CompareData(expected, indicator, data3, ret);

    ret = SQLGetData(m_hstmt, 4, SQL_C_WCHAR, data4, 1024, &indicator);
    expected = CREATE_STRING("-0-1");
    CompareData(expected, indicator, data4, ret);

    ret = SQLGetData(m_hstmt, 5, SQL_C_WCHAR, data5, 1024, &indicator);
    expected = CREATE_STRING("0-0");
    CompareData(expected, indicator, data5, ret);

    ret = SQLGetData(m_hstmt, 6, SQL_C_WCHAR, data6, 1024, &indicator);
    expected = CREATE_STRING("0-0");
    CompareData(expected, indicator, data6, ret);

    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTERVAL_DAY_TO_SECOND_TO_SQL_C_CHAR) {
    test_string columns = CREATE_STRING("1d,1h,1m,1s,1ms,1us,1ns");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLCHAR data2[1024] = {0};
    SQLCHAR data3[1024] = {0};
    SQLCHAR data4[1024] = {0};
    SQLCHAR data5[1024] = {0};
    SQLCHAR data6[1024] = {0};
    SQLCHAR data7[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected;
    expected = "1 00:00:00.000000000";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data2, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "0 01:00:00.000000000";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data2);
    ret = SQLGetData(m_hstmt, 3, SQL_C_CHAR, data3, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "0 00:01:00.000000000";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data3);
    ret = SQLGetData(m_hstmt, 4, SQL_C_CHAR, data4, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "0 00:00:01.000000000";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data4);
    ret = SQLGetData(m_hstmt, 5, SQL_C_CHAR, data5, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "0 00:00:00.001000000";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data5);
    ret = SQLGetData(m_hstmt, 6, SQL_C_CHAR, data6, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "0 00:00:00.000001000";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data6);
    ret = SQLGetData(m_hstmt, 7, SQL_C_CHAR, data7, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "0 00:00:00.000000001";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data7);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTERVAL_DAY_TO_SECOND_TO_SQL_C_WCHAR) {
    test_string columns = CREATE_STRING("1d,1h,1m,1s,1ms,1us,1ns");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR data[1024] = {0};
    SQLTCHAR data2[1024] = {0};
    SQLTCHAR data3[1024] = {0};
    SQLTCHAR data4[1024] = {0};
    SQLTCHAR data5[1024] = {0};
    SQLTCHAR data6[1024] = {0};
    SQLTCHAR data7[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    test_string expected = CREATE_STRING("1 00:00:00.000000000");
    CompareData(expected, indicator, data, ret);

    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data2, 1024, &indicator);
    expected = CREATE_STRING("0 01:00:00.000000000");
    CompareData(expected, indicator, data2, ret);

    ret = SQLGetData(m_hstmt, 3, SQL_C_WCHAR, data3, 1024, &indicator);
    expected = CREATE_STRING("0 00:01:00.000000000");
    CompareData(expected, indicator, data3, ret);

    ret = SQLGetData(m_hstmt, 4, SQL_C_WCHAR, data4, 1024, &indicator);
    expected = CREATE_STRING("0 00:00:01.000000000");
    CompareData(expected, indicator, data4, ret);

    ret = SQLGetData(m_hstmt, 5, SQL_C_WCHAR, data5, 1024, &indicator);
    expected = CREATE_STRING("0 00:00:00.001000000");
    CompareData(expected, indicator, data5, ret);
    
    ret = SQLGetData(m_hstmt, 6, SQL_C_WCHAR, data6, 1024, &indicator);
    expected = CREATE_STRING("0 00:00:00.000001000");
    CompareData(expected, indicator, data6, ret);
    
    ret = SQLGetData(m_hstmt, 7, SQL_C_WCHAR, data7, 1024, &indicator);
    expected = CREATE_STRING("0 00:00:00.000000001");
    CompareData(expected, indicator, data7, ret);
    
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLNumResultCols, SingleColumn) {
    test_string columns =
        CREATE_STRING("Array[Row(null), Row(NULL)]");
    ExecuteQuery(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT column_count = 0;
    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(m_hstmt, &column_count));
    EXPECT_EQ(1, column_count);
}

TEST_F(TestSQLNumResultCols, MultiColumn) {
    test_string columns =
        CREATE_STRING("Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL]), ")
        CREATE_STRING("Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL]), ")
        CREATE_STRING("Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL]), ")
        CREATE_STRING("Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL]), ")
        CREATE_STRING("Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL]), ")
        CREATE_STRING("Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL]), ")
        CREATE_STRING("Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL])");
    ExecuteQuery(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT column_count = 0;
    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(m_hstmt, &column_count));
    EXPECT_EQ(14, column_count);
}

TEST_F(TestSQLDescribeCol, INTEGER_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("INTEGER \'1\'");
    ExecuteQuery(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_INTEGER, data_type);
    EXPECT_EQ((SQLULEN)11, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, DOUBLE_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("DOUBLE \'1.0\'");
    ExecuteQuery(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_DOUBLE, data_type);
    EXPECT_EQ((SQLULEN)15, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, BIGINT_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("BIGINT \'2147483648\'");
    ExecuteQuery(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_BIGINT, data_type);
    EXPECT_EQ((SQLULEN)20, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, BOOLEAN_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("true");
    ExecuteQuery(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_BIT, data_type);
    EXPECT_EQ((SQLULEN)1, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, VARCHAR_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("VARCHAR\'ABCDEFG\'");
    ExecuteQuery(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, TIMESERIES_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string statement =
        CREATE_STRING("WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 ")
        CREATE_STRING("14:18:30.123456789' AS binned_timestamp, ROW(null, ARRAY[ARRAY[ROW(12345, ARRAY[1, 2, 3])]]) ")
        CREATE_STRING("AS data FROM ODBCTest.IoT LIMIT ")CREATE_STRING("1), interpolated_timeseries AS(SELECT ")
        CREATE_STRING("CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) ")
        CREATE_STRING("SELECT *FROM interpolated_timeseries");
    ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)statement.c_str(),
                        (SQLINTEGER)statement.length());
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    ret = SQLFetch(m_hstmt);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, ARRAY_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns =
        CREATE_STRING("ARRAY[ARRAY[ARRAY[ARRAY[1.1, 2.3], ARRAY[1.1, 2.3]]], ARRAY[ARRAY[ARRAY[1.1, 2.3], ARRAY[1.1, 2.3]]]]");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, ROW_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("ROW(ROW(ROW(INTEGER '03', BIGINT '10', true), ARRAY[ARRAY[1,2],ARRAY[1.1,2.2]]))");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, NULL_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("null");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, TIMESTAMP_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("TIMESTAMP \'2021-01-02 18:01:13.000000000\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_TYPE_TIMESTAMP, data_type);
    std::string expected = "2021-01-02 18:01:13.000000000";
    EXPECT_EQ((SQLULEN)expected.size(), column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, DATE_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("DATE \'2021-01-02\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_TYPE_DATE, data_type);
    std::string expected = "2021-01-02";
    EXPECT_EQ((SQLULEN)expected.size(), column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, TIME_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("TIME \'06:39:45.123456789\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_TYPE_TIME, data_type);
    std::string expected = "06:39:45.123456789";
    EXPECT_EQ((SQLULEN)expected.size(), column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, INTERVAL_YEAR_TO_MONTH_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("1year");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, INTERVAL_DAY_TO_SECOND_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("1d");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, OUT_OF_INDEX_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("INTEGER\'1\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 2, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_INVALID_DESCRIPTOR_INDEX));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, TRUNCATED_COLUMN_NAME_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("INTEGER\'1\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[1];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 1, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_DATA_RIGHT_TRUNCATED));
    test_string truncated_column_name = CREATE_STRING("_");
    EXPECT_EQ(truncated_column_name[0], column_name[0]);
    test_string expected_column_name = CREATE_STRING("_col0");
    EXPECT_EQ((SQLSMALLINT)expected_column_name.size(), column_name_length);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, INVALID_STRING_OR_BUFFER_LENGTH) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("INTEGER\'1\'");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, -1, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_INVALID_STRING_OR_BUFFER_LENGTH));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeCol, MULTIPLE_COLUMNS) {
    SQLRETURN ret = SQL_ERROR;
    test_string columns = CREATE_STRING("INTEGER\'1\', DOUBLE \'1.0\', BIGINT \'2147483648\', true");
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_INTEGER, data_type);
    EXPECT_EQ((SQLULEN)11, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    ret = SQLDescribeCol(m_hstmt, 2, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected_column_name = "_col1";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_DOUBLE, data_type);
    EXPECT_EQ((SQLULEN)15, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    ret = SQLDescribeCol(m_hstmt, 3, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected_column_name = "_col2";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_BIGINT, data_type);
    EXPECT_EQ((SQLULEN)20, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    ret = SQLDescribeCol(m_hstmt, 4, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected_column_name = "_col3";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_BIT, data_type);
    EXPECT_EQ((SQLULEN)1, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLMoreResults, NoData_noquery) {
    SQLRETURN ret = SQLMoreResults(m_hstmt);
    EXPECT_EQ(SQL_NO_DATA, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLMoreResults, NoData_query) {
    test_string columns = CREATE_STRING("Array[Row(null), Row(NULL)]");
    ExecuteQuery(columns, table_name, CREATE_STRING("100"), &m_hstmt);
    SQLRETURN ret = SQLMoreResults(m_hstmt);
    EXPECT_EQ(SQL_NO_DATA, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

// Row count is not supported for the driver, so this should return -1,
// as defined in the ODBC API.
TEST_F(TestSQLRowCount, RowCountNotAvailable) {
    SQLLEN row_count = 0;
    SQLRETURN ret = SQLRowCount(m_hstmt, &row_count);
    EXPECT_EQ(SQL_SUCCESS, ret);
    EXPECT_EQ(row_count, -1L);
}

TEST_F(TestSQLRowCount, InvalidHandle) {
    SQLLEN row_count = 0;
    SQLRETURN ret = SQLRowCount(nullptr, &row_count);
    EXPECT_EQ(SQL_INVALID_HANDLE, ret);
    EXPECT_EQ(row_count, -1L);
}

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
    ::testing::InitGoogleTest(&argc, argv);

    int failures = RUN_ALL_TESTS();

    std::string output = "No output.";
    std::cout << output << std::endl;
    std::cout << (failures ? "Not all tests passed." : "All tests passed")
              << std::endl;
    WriteFileIfSpecified(argv, argv + argc, "-fout", output);

#ifdef __APPLE__
    // Disable malloc logging and report memory leaks
    system("unset MallocStackLogging");
    system("leaks itodbc_results > leaks_itodbc_results");
#endif
    return failures;
}
