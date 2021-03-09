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
#include "pch.h"
#include "unit_test_helper.h"
#include "it_odbc_helper.h"
#include <cmath>
// clang-format on

//typedef struct DescribeColumnData {
//    std::wstring column_name;
//    SQLSMALLINT data_type;
//} DescribeColumnData;
//
//const DescribeColumnData column_data[] = {{L"Origin", SQL_WVARCHAR},
//                                          {L"FlightNum", SQL_WVARCHAR},
//                                          {L"FlightDelay", SQL_BIT},
//                                          {L"DistanceMiles", SQL_REAL},
//                                          {L"FlightTimeMin", SQL_REAL},
//                                          {L"OriginWeather", SQL_WVARCHAR},
//                                          {L"dayOfWeek", SQL_INTEGER},
//                                          {L"AvgTicketPrice", SQL_REAL},
//                                          {L"Carrier", SQL_WVARCHAR},
//                                          {L"FlightDelayMin", SQL_INTEGER},
//                                          {L"OriginRegion", SQL_WVARCHAR},
//                                          {L"DestAirportID", SQL_WVARCHAR},
//                                          {L"FlightDelayType", SQL_WVARCHAR},
//                                          {L"timestamp", SQL_TYPE_TIMESTAMP},
//                                          {L"Dest", SQL_WVARCHAR},
//                                          {L"FlightTimeHour", SQL_WVARCHAR},
//                                          {L"Cancelled", SQL_BIT},
//                                          {L"DistanceKilometers", SQL_REAL},
//                                          {L"OriginCityName", SQL_WVARCHAR},
//                                          {L"DestWeather", SQL_WVARCHAR},
//                                          {L"OriginCountry", SQL_WVARCHAR},
//                                          {L"DestCountry", SQL_WVARCHAR},
//                                          {L"DestRegion", SQL_WVARCHAR},
//                                          {L"OriginAirportID", SQL_WVARCHAR},
//                                          {L"DestCityName", SQL_WVARCHAR}};
const std::wstring table_name = L"ODBCTest.IoT";
//const std::wstring multi_type_data_set = L"kibana_sample_data_types";
//const std::wstring single_col = L"Origin";
//// TODO (#110): Improve sample data result checks
//const std::wstring m_expected_origin_column_data_1 =
//    L"Frankfurt am Main Airport";
//const std::wstring m_expected_origin_column_data_2 = L"Olenya Air Base";
//const std::wstring single_float_col = L"DistanceMiles";
//const std::wstring single_integer_col = L"FlightDelayMin";
//const std::wstring single_timestamp_col = L"timestamp";
//const std::wstring single_bit_col = L"Cancelled";
//const std::wstring single_row_offset_3 = L"1 OFFSET 3";
//const uint32_t data_cnt = 2;
//const std::wstring single_row_offsets[data_cnt] = {L"1", L"1 OFFSET 1"};
//const std::wstring type_boolean = L"type_boolean";
//const std::vector< bool > type_boolean_vals = {false, true, true};
//const std::wstring type_byte = L"type_byte";
//const std::vector< int16_t > type_byte_vals = {100, -120};
//const std::wstring type_short = L"type_short";
//const std::vector< int16_t > type_short_vals = {1000, -2000};
//const std::wstring type_integer = L"type_integer";
//const std::vector< int32_t > type_integer_vals = {250000000, -350000000};
//const std::wstring type_long = L"type_long";
//const std::vector< int64_t > type_long_vals = {8000000000, -8010000000};
//const std::wstring type_half_float = L"type_half_float";
//const std::vector< float > type_half_float_vals = {1.115f, -2.115f};
//const std::wstring type_float = L"type_float";
//const std::vector< float > type_float_vals = {2.1512f, -3.1512f};
//const std::wstring type_double = L"type_double";
//const std::vector< double > type_double_vals = {25235.2215, -5335.2215};
//const std::wstring type_scaled_float = L"type_scaled_float";
//const std::vector< double > type_scaled_float_vals = {100, -100.1};
//const std::wstring type_keyword = L"type_keyword";
//const std::vector< std::wstring > type_keyword_vals = {L"hello", L"goodbye"};
//const std::wstring type_text = L"type_text";
//const std::vector< std::wstring > type_text_vals = {L"world", L"planet"};
//const std::wstring type_date = L"type_date";
//const std::vector< TIMESTAMP_STRUCT > type_date_vals = {
//    {2016, 02, 21, 12, 23, 52, 803000000},
//    {2018, 07, 22, 12, 23, 52, 803000000}};
//const std::wstring type_object = L"type_object";
//const std::wstring type_nested = L"type_nested";
//// TODO (#110): Improve sample data result checks
//const float distance_miles_1 = 1738.98f;
//const float distance_miles_2 = 10247.90f;
//const int delay_offset_3_1 = 0;
//const int delay_offset_3_2 = 180;
//const SQLSMALLINT single_col_name_length = 6;
//const SQLSMALLINT single_col_data_type = SQL_WVARCHAR;
//const SQLULEN single_col_column_size = 25;
//const SQLSMALLINT single_col_decimal_digit = 0;
//const SQLSMALLINT single_col_nullable = 2;
const std::wstring single_row = L"1";
//const size_t multi_row_cnt = 25;
//const size_t single_row_cnt = 1;
//const size_t multi_col_cnt = 25;
//const size_t single_col_cnt = 1;
//const size_t single_row_rd_cnt = 1;
//const size_t multi_row_rd_cnt_aligned = 5;
//const size_t multi_row_rd_cnt_misaligned = 3;
//const std::wstring multi_col = L"*";
//const std::wstring multi_row = std::to_wstring(multi_row_cnt);
//typedef struct Col {
//    SQLLEN data_len;
//    SQLCHAR data_dat[255];
//} Col;

//template < class T >
//void CheckData(const std::wstring& type_name, const std::wstring& data_set,
//               const std::wstring row, SQLHSTMT* hstmt,
//               const SQLUSMALLINT ordinal_pos, const SQLUSMALLINT type,
//               const std::vector< T >& expected_val, const SQLLEN data_size);
//template < class T >
//inline bool FuzzyEquals(T a, T b, T epsil);
//void BindColumns(std::vector< std::vector< Col > >& cols, SQLHSTMT* hstmt);
// void ExecuteQuery(const std::wstring& column, const std::wstring& dataset,
//                   const std::wstring& count, SQLHSTMT* hstmt);
//void ExtendedFetch(const size_t exp_row_cnt, const size_t exp_read_cnt,
//                   const bool aligned, const size_t total_row_cnt,
//                   SQLHSTMT* hstmt);
//void Fetch(size_t exp_row_cnt, SQLHSTMT* hstmt);
//void QueryBind(const size_t row_cnt, const size_t col_cnt,
//               const size_t row_fetch_cnt, const std::wstring& column_name,
//               std::vector< std::vector< Col > >& cols, SQLHSTMT* hstmt);
//void QueryBindFetch(const size_t row_cnt, const size_t col_cnt,
//                    const std::wstring& column_name, SQLHSTMT* hstmt);
// void QueryFetch(const std::wstring& column, const std::wstring& dataset,
//                 const std::wstring& count, SQLHSTMT* hstmt);

//template < class T >
//inline bool FuzzyEquals(T a, T b, T epsil) {
//    return std::abs(a - b) <= epsil;
//}

//inline void BindColumns(std::vector< std::vector< Col > >& cols,
//                        SQLHSTMT* hstmt) {
//    SQLRETURN ret;
//    for (size_t i = 0; i < cols.size(); i++) {
//        ret = SQLBindCol(*hstmt, (SQLUSMALLINT)i + 1, SQL_C_CHAR,
//                         (SQLPOINTER)&cols[i][0].data_dat[i], 255,
//                         &cols[i][0].data_len);
//        LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
//        ASSERT_TRUE(SQL_SUCCEEDED(ret));
//    }
//}

void ExecuteQuery(const std::wstring& column,
                         const std::wstring& table, const std::wstring& count,
                         SQLHSTMT* hstmt) {
    std::wstring statement = QueryBuilder(column, table, count);
    SQLRETURN ret = SQLExecDirect(*hstmt, (SQLTCHAR*)statement.c_str(),
                                  (SQLINTEGER)statement.length());
    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
}
//
//inline void ExtendedFetch(const size_t exp_row_cnt, const size_t exp_read_cnt,
//                          const bool aligned, const size_t total_row_cnt,
//                          SQLHSTMT* hstmt) {
//    SQLULEN row_cnt = 0;
//    SQLUSMALLINT row_stat[10];
//    size_t read_cnt = 0;
//    SQLRETURN ret;
//    while (
//        (ret = SQLExtendedFetch(*hstmt, SQL_FETCH_NEXT, 0, &row_cnt, row_stat))
//        == SQL_SUCCESS) {
//        read_cnt++;
//        if (aligned) {
//            EXPECT_EQ(row_cnt, exp_row_cnt);
//        } else {
//            size_t adj_exp_row_cnt = ((read_cnt * exp_row_cnt) > total_row_cnt)
//                                         ? (total_row_cnt % exp_row_cnt)
//                                         : exp_row_cnt;
//            EXPECT_EQ(row_cnt, adj_exp_row_cnt);
//        }
//    }
//
//    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
//    EXPECT_EQ(exp_read_cnt, read_cnt);
//}
//
//inline void Fetch(size_t exp_row_cnt, SQLHSTMT* hstmt) {
//    SQLRETURN ret;
//    size_t read_cnt = 0;
//    while ((ret = SQLFetch(*hstmt)) == SQL_SUCCESS) {
//        read_cnt++;
//    }
//
//    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
//    EXPECT_EQ(exp_row_cnt, read_cnt);
//}

//inline void QueryBind(const size_t row_cnt, const size_t col_cnt,
//                      const size_t row_fetch_cnt,
//                      const std::wstring& column_name,
//                      std::vector< std::vector< Col > >& cols,
//                      SQLHSTMT* hstmt) {
//    (void)col_cnt;
//    SQLRETURN ret =
//        SQLSetStmtAttr(*hstmt, SQL_ROWSET_SIZE, (void*)row_fetch_cnt, 0);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
//    ASSERT_EQ(ret, SQL_SUCCESS);
//
//    std::wstring row_str = std::to_wstring(row_cnt);
//    ExecuteQuery(column_name, flight_data_set, row_str, hstmt);
//
//    for (size_t i = 0; i < cols.size(); i++) {
//        cols[i].resize(row_fetch_cnt);
//    }
//    BindColumns(cols, hstmt);
//}

//inline void QueryBindExtendedFetch(const size_t row_cnt, const size_t col_cnt,
//                                   const size_t row_fetch_cnt,
//                                   const std::wstring& column_name,
//                                   SQLHSTMT* hstmt) {
//    std::vector< std::vector< Col > > cols(col_cnt);
//    QueryBind(row_cnt, col_cnt, row_fetch_cnt, column_name, cols, hstmt);
//
//    // Fetch data
//    size_t misaligned = ((row_cnt % row_fetch_cnt) != 0);
//    size_t exp_read_cnt = (row_cnt / row_fetch_cnt) + misaligned;
//    ExtendedFetch(row_fetch_cnt, exp_read_cnt, (bool)!misaligned, row_cnt,
//                  hstmt);
//    CloseCursor(hstmt, true, true);
//}
//
//inline void QueryBindFetch(const size_t row_cnt, const size_t col_cnt,
//                           const std::wstring& column_name, SQLHSTMT* hstmt) {
//    std::vector< std::vector< Col > > cols(col_cnt);
//    QueryBind(row_cnt, col_cnt, 1, column_name, cols, hstmt);
//
//    // Fetch data
//    Fetch(row_cnt, hstmt);
//    CloseCursor(hstmt, true, true);
//}
//
void QueryFetch(const std::wstring& column, const std::wstring& dataset,
                const std::wstring& count, SQLHSTMT* hstmt) {
    ExecuteQuery(column, dataset, count, hstmt);
    SQLRETURN ret = SQLFetch(*hstmt);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
}
//
//template < class T >
//void CheckData(const std::wstring& type_name, const std::wstring& data_set,
//               const std::wstring row, SQLHSTMT* hstmt,
//               const SQLUSMALLINT ordinal_pos, const SQLUSMALLINT type,
//               const std::vector< T >& expected_val, const SQLLEN data_size) {
//    QueryFetch(type_name, data_set, row, hstmt);
//    T val;
//    SQLLEN out_size;
//    const SQLRETURN ret =
//        SQLGetData(*hstmt, ordinal_pos, type, &val, data_size, &out_size);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
//    ASSERT_TRUE(SQL_SUCCEEDED(ret));
//    bool valid = false;
//    for (size_t i = 0; i < expected_val.size(); i++) {
//        valid |= (val == expected_val[i]);
//        if (valid)
//            break;
//    }
//    EXPECT_TRUE(valid);
//}
//
//template <>
//void CheckData< float >(const std::wstring& type_name,
//                        const std::wstring& data_set, const std::wstring row,
//                        SQLHSTMT* hstmt, const SQLUSMALLINT ordinal_pos,
//                        const SQLUSMALLINT type,
//                        const std::vector< float >& expected_val,
//                        const SQLLEN data_size) {
//    QueryFetch(type_name, data_set, row, hstmt);
//    float val;
//    SQLLEN out_size;
//    const SQLRETURN ret =
//        SQLGetData(*hstmt, ordinal_pos, type, &val, data_size, &out_size);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
//    ASSERT_TRUE(SQL_SUCCEEDED(ret));
//    bool valid = false;
//    for (auto& it : expected_val) {
//        valid |= FuzzyEquals(val, it, 0.1f);
//        if (valid)
//            break;
//    }
//}
//
//template <>
//void CheckData< double >(const std::wstring& type_name,
//                         const std::wstring& data_set, const std::wstring row,
//                         SQLHSTMT* hstmt, const SQLUSMALLINT ordinal_pos,
//                         const SQLUSMALLINT type,
//                         const std::vector< double >& expected_val,
//                         const SQLLEN data_size) {
//    QueryFetch(type_name, data_set, row, hstmt);
//    double val;
//    SQLLEN out_size;
//    const SQLRETURN ret =
//        SQLGetData(*hstmt, ordinal_pos, type, &val, data_size, &out_size);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
//    ASSERT_TRUE(SQL_SUCCEEDED(ret));
//    bool valid = false;
//    for (auto& it : expected_val) {
//        valid |= FuzzyEquals(val, it, 0.1);
//        if (valid)
//            break;
//    }
//    EXPECT_TRUE(valid);
//}
//
//template <>
//void CheckData< std::wstring >(const std::wstring& type_name,
//                               const std::wstring& data_set,
//                               const std::wstring row, SQLHSTMT* hstmt,
//                               const SQLUSMALLINT ordinal_pos,
//                               const SQLUSMALLINT type,
//                               const std::vector< std::wstring >& expected_val,
//                               const SQLLEN data_size) {
//    QueryFetch(type_name, data_set, row, hstmt);
//    std::vector< SQLTCHAR > val(data_size);
//    bool valid = false;
//    for (auto& it : expected_val) {
//        std::wstring str;
//        SQLLEN out_size;
//        while (SQLGetData(*hstmt, ordinal_pos, type, val.data(),
//                          data_size * sizeof(SQLTCHAR), &out_size)
//               == SQL_SUCCESS_WITH_INFO) {
//            str += val.data();
//        }
//        valid |= (str == it);
//        if (valid)
//            break;
//    }
//}
//
//template <>
//void CheckData< TIMESTAMP_STRUCT >(
//    const std::wstring& type_name, const std::wstring& data_set,
//    const std::wstring row, SQLHSTMT* hstmt, const SQLUSMALLINT ordinal_pos,
//    const SQLUSMALLINT type,
//    const std::vector< TIMESTAMP_STRUCT >& expected_val,
//    const SQLLEN data_size) {
//    auto compare_ts_struct = [](const TIMESTAMP_STRUCT& x,
//                                const TIMESTAMP_STRUCT& y) {
//        return ((x.year == y.year) && (x.month == y.month) && (x.day == y.day)
//                && (x.hour == y.hour) && (x.minute == y.minute)
//                && (x.second == y.second) && (x.fraction == y.fraction));
//    };
//    QueryFetch(type_name, data_set, row, hstmt);
//    TIMESTAMP_STRUCT val;
//    SQLLEN out_size;
//    const SQLRETURN ret =
//        SQLGetData(*hstmt, ordinal_pos, type, &val, data_size, &out_size);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
//    ASSERT_TRUE(SQL_SUCCEEDED(ret));
//    bool valid = false;
//    for (size_t i = 0; i < expected_val.size(); i++) {
//        valid |= compare_ts_struct(val, expected_val[i]);
//        if (valid)
//            break;
//    }
//    EXPECT_TRUE(valid);
//}
//
//#define GET_DATA_TEST(type, name, c_type, val_array, sz)                \
//    TEST_F(TestSQLGetData, type) {                                      \
//        for (uint32_t i = 0; i < data_cnt; i++) {                       \
//            CheckData(name, multi_type_data_set, single_row_offsets[i], \
//                      &m_hstmt, m_single_column_ordinal_position,       \
//                      (SQLUSMALLINT)c_type, val_array, sz);             \
//            if (i != (data_cnt - 1))                                    \
//                ASSERT_NO_THROW(CloseCursor(&m_hstmt, true, true));     \
//        }                                                               \
//    }
//
//class TestSQLBindCol : public testing::Test {
//   public:
//    TestSQLBindCol() {
//    }
//
//    void SetUp() {
//        ASSERT_NO_THROW(AllocStatement((SQLTCHAR*)conn_string.c_str(), &m_env,
//                                       &m_conn, &m_hstmt, true, true));
//    }
//
//    void TearDown() {
//        ASSERT_NO_THROW(CloseCursor(&m_hstmt, true, true));
//        SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
//        SQLDisconnect(m_conn);
//        SQLFreeHandle(SQL_HANDLE_ENV, m_env);
//    }
//
//    ~TestSQLBindCol() {
//        // cleanup any pending stuff, but no exceptions allowed
//    }
//
//    SQLHENV m_env = SQL_NULL_HENV;
//    SQLHDBC m_conn = SQL_NULL_HDBC;
//    SQLHSTMT m_hstmt = SQL_NULL_HSTMT;
//};
//
//class TestSQLFetch : public testing::Test {
//   public:
//    TestSQLFetch() {
//    }
//
//    void SetUp() {
//        ASSERT_NO_THROW(AllocStatement((SQLTCHAR*)conn_string.c_str(), &m_env,
//                                       &m_conn, &m_hstmt, true, true));
//    }
//
//    void TearDown() {
//        if (m_hstmt != SQL_NULL_HSTMT) {
//            ASSERT_NO_THROW(CloseCursor(&m_hstmt, true, true));
//            SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
//            SQLDisconnect(m_conn);
//            SQLFreeHandle(SQL_HANDLE_ENV, m_env);
//        }
//    }
//
//    ~TestSQLFetch() {
//        // cleanup any pending stuff, but no exceptions allowed
//    }
//
//    SQLHENV m_env = SQL_NULL_HENV;
//    SQLHDBC m_conn = SQL_NULL_HDBC;
//    SQLHSTMT m_hstmt = SQL_NULL_HSTMT;
//};
//
//class TestSQLExtendedFetch : public testing::Test {
//   public:
//    TestSQLExtendedFetch() {
//    }
//
//    void SetUp() {
//        ASSERT_NO_THROW(AllocStatement((SQLTCHAR*)conn_string.c_str(), &m_env,
//                                       &m_conn, &m_hstmt, true, true));
//    }
//
//    void TearDown() {
//        if (m_hstmt != SQL_NULL_HSTMT) {
//            ASSERT_NO_THROW(CloseCursor(&m_hstmt, true, true));
//            SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
//            SQLDisconnect(m_conn);
//            SQLFreeHandle(SQL_HANDLE_ENV, m_env);
//        }
//    }
//
//    ~TestSQLExtendedFetch() {
//        // cleanup any pending stuff, but no exceptions allowed
//    }
//
//    SQLHENV m_env = SQL_NULL_HENV;
//    SQLHDBC m_conn = SQL_NULL_HDBC;
//    SQLHSTMT m_hstmt = SQL_NULL_HSTMT;
//};

class Fixture : public testing::Test {
   public:
    void SetUp() {
        ASSERT_NO_THROW(AllocStatement((SQLTCHAR*)conn_string.c_str(), &m_env,
                                       &m_conn, &m_hstmt, true, true));
    }
    void TearDown() {
        ASSERT_NO_THROW(CloseCursor(&m_hstmt, true, true));
        SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
        SQLDisconnect(m_conn);
        SQLFreeHandle(SQL_HANDLE_ENV, m_env);
    }
    SQLHENV m_env = SQL_NULL_HENV;
    SQLHDBC m_conn = SQL_NULL_HDBC;
    SQLHSTMT m_hstmt = SQL_NULL_HSTMT;
};

class TestSQLGetData : public Fixture {};

class TestSQLNumResultCols : public Fixture {};

//class TestSQLMoreResults : public testing::Test {
//   public:
//    TestSQLMoreResults() {
//    }
//
//    void SetUp() {
//        ASSERT_NO_THROW(AllocStatement((SQLTCHAR*)conn_string.c_str(), &m_env,
//                                       &m_conn, &m_hstmt, true, true));
//    }
//
//    void TearDown() {
//        if (m_hstmt != SQL_NULL_HSTMT) {
//            ASSERT_NO_THROW(CloseCursor(&m_hstmt, true, true));
//            SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
//            SQLDisconnect(m_conn);
//            SQLFreeHandle(SQL_HANDLE_ENV, m_env);
//        }
//    }
//
//    ~TestSQLMoreResults() {
//        // cleanup any pending stuff, but no exceptions allowed
//    }
//
//    SQLHENV m_env = SQL_NULL_HENV;
//    SQLHDBC m_conn = SQL_NULL_HDBC;
//    SQLHSTMT m_hstmt = SQL_NULL_HSTMT;
//};
//
//class TestSQLDescribeCol : public testing::Test {
//   public:
//    TestSQLDescribeCol() {
//    }
//
//    void SetUp() {
//        ASSERT_NO_THROW(AllocStatement((SQLTCHAR*)conn_string.c_str(), &m_env,
//                                       &m_conn, &m_hstmt, true, true));
//    }
//
//    void TearDown() {
//        if (m_hstmt != SQL_NULL_HSTMT) {
//            ASSERT_NO_THROW(CloseCursor(&m_hstmt, true, true));
//            SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
//            SQLDisconnect(m_conn);
//            SQLFreeHandle(SQL_HANDLE_ENV, m_env);
//        }
//    }
//
//    ~TestSQLDescribeCol() {
//        // cleanup any pending stuff, but no exceptions allowed
//    }
//
//    SQLHENV m_env = SQL_NULL_HENV;
//    SQLHDBC m_conn = SQL_NULL_HDBC;
//    SQLHSTMT m_hstmt = SQL_NULL_HSTMT;
//    SQLSMALLINT m_column_number;
//    SQLTCHAR m_column_name[50];
//    SQLSMALLINT m_column_name_length;
//    SQLSMALLINT m_data_type;
//    SQLULEN m_column_size;
//    SQLSMALLINT m_decimal_digits;
//    SQLSMALLINT m_nullable;
//};
//
//class TestSQLRowCount : public testing::Test {
//   public:
//    TestSQLRowCount() {
//    }
//
//    void SetUp() {
//        ASSERT_NO_THROW(AllocStatement((SQLTCHAR*)conn_string.c_str(), &m_env,
//                                       &m_conn, &m_hstmt, true, true));
//    }
//
//    void TearDown() {
//        if (m_hstmt != SQL_NULL_HSTMT) {
//            ASSERT_NO_THROW(CloseCursor(&m_hstmt, true, true));
//            SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
//            SQLDisconnect(m_conn);
//            SQLFreeHandle(SQL_HANDLE_ENV, m_env);
//        }
//    }
//
//    ~TestSQLRowCount() {
//        // cleanup any pending stuff, but no exceptions allowed
//    }
//
//    SQLHENV m_env = SQL_NULL_HENV;
//    SQLHDBC m_conn = SQL_NULL_HDBC;
//    SQLHSTMT m_hstmt = SQL_NULL_HSTMT;
//};
//
//TEST_F(TestSQLBindCol, SingleColumnSingleBind) {
//    std::vector< std::vector< Col > > cols(single_col_cnt);
//    QueryBind(single_row_cnt, single_col_cnt, 1, single_col, cols, &m_hstmt);
//}
//
//TEST_F(TestSQLBindCol, MultiColumnMultiBind) {
//    std::vector< std::vector< Col > > cols(multi_col_cnt);
//    QueryBind(single_row_cnt, multi_col_cnt, 1, multi_col, cols, &m_hstmt);
//}
//
//// Looked at SQLBindCol - if < requested column are allocated, it will
//// reallocate additional space for that column
//TEST_F(TestSQLBindCol, InvalidColIndex0) {
//    std::vector< std::vector< Col > > cols(single_col_cnt);
//    SQLRETURN ret = SQLSetStmtAttr(m_hstmt, SQL_ROWSET_SIZE, (void*)1, 0);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//    ASSERT_EQ(ret, SQL_SUCCESS);
//
//    std::wstring row_str = std::to_wstring(single_row_cnt);
//    ExecuteQuery(single_col, flight_data_set, row_str, &m_hstmt);
//
//    for (size_t i = 0; i < cols.size(); i++) {
//        cols[i].resize(1);
//    }
//    ret = SQLBindCol(m_hstmt, (SQLUSMALLINT)1, SQL_C_CHAR,
//                     (SQLPOINTER)&cols[0][0].data_dat[0], 255,
//                     &cols[0][0].data_len);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//    ASSERT_TRUE(SQL_SUCCEEDED(ret));
//    ret = SQLBindCol(m_hstmt, (SQLUSMALLINT)0, SQL_C_CHAR,
//                     (SQLPOINTER)&cols[0][0].data_dat[0], 255,
//                     &cols[0][0].data_len);
//    EXPECT_FALSE(SQL_SUCCEEDED(ret));
//}
//
//TEST_F(TestSQLBindCol, InsufficientSpace) {
//    SQLRETURN ret = SQLSetStmtAttr(m_hstmt, SQL_ROWSET_SIZE, (void*)1, 0);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//    ASSERT_EQ(ret, SQL_SUCCESS);
//
//    std::wstring row_str = std::to_wstring(single_row_cnt);
//    ExecuteQuery(single_col, flight_data_set, row_str, &m_hstmt);
//
//    SQLLEN length = 0;
//    std::vector< SQLTCHAR > data_buffer(2);
//    ret = SQLBindCol(m_hstmt, (SQLUSMALLINT)1, SQL_C_CHAR,
//                     (SQLPOINTER)data_buffer.data(), 2, &length);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//    ASSERT_TRUE(SQL_SUCCEEDED(ret));
//
//    SQLULEN row_cnt = 0;
//    SQLUSMALLINT row_stat = 0;
//    std::vector< SQLTCHAR > msg_buffer(512);
//    ret = SQLExtendedFetch(m_hstmt, SQL_FETCH_NEXT, 0, &row_cnt, &row_stat);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret, msg_buffer.data(), 512);
//    EXPECT_EQ(ret, SQL_SUCCESS_WITH_INFO);
//    EXPECT_STREQ(msg_buffer.data(), L"Fetched item was truncated.");
//    // TODO (#110): Improve sample data result checks
//    const wchar_t* data =
//        reinterpret_cast< const wchar_t* >(data_buffer.data());
//    bool found_expected_data =
//        wcscmp(data, m_expected_origin_column_data_1.substr(0, 1).c_str())
//        || wcscmp(data, m_expected_origin_column_data_2.substr(0, 1).c_str());
//    EXPECT_TRUE(found_expected_data);
//}
//
//TEST_F(TestSQLFetch, SingleCol_SingleRow) {
//    EXPECT_NO_THROW(
//        QueryBindFetch(single_row_cnt, single_col_cnt, single_col, &m_hstmt));
//}
//
//TEST_F(TestSQLFetch, SingleCol_MultiRow) {
//    EXPECT_NO_THROW(
//        QueryBindFetch(multi_row_cnt, single_col_cnt, single_col, &m_hstmt));
//}
//
//TEST_F(TestSQLFetch, MultiCol_SingleRow) {
//    EXPECT_NO_THROW(
//        QueryBindFetch(single_row_cnt, multi_col_cnt, multi_col, &m_hstmt));
//}
//
//TEST_F(TestSQLFetch, MultiCol_MultiRow) {
//    EXPECT_NO_THROW(
//        QueryBindFetch(multi_row_cnt, multi_col_cnt, multi_col, &m_hstmt));
//}
//
//TEST_F(TestSQLExtendedFetch, SingleCol_SingleRow) {
//    EXPECT_NO_THROW(QueryBindExtendedFetch(single_row_cnt, single_col_cnt,
//                                           single_row_rd_cnt, single_col,
//                                           &m_hstmt));
//}
//
//TEST_F(TestSQLExtendedFetch, SingleCol_MultiRow_SingleFetch) {
//    EXPECT_NO_THROW(QueryBindExtendedFetch(multi_row_cnt, single_col_cnt,
//                                           single_row_rd_cnt, single_col,
//                                           &m_hstmt));
//}
//
//TEST_F(TestSQLExtendedFetch, SingleCol_MultiRow_MultiFetch_Aligned) {
//    EXPECT_NO_THROW(QueryBindExtendedFetch(multi_row_cnt, single_col_cnt,
//                                           multi_row_rd_cnt_aligned, single_col,
//                                           &m_hstmt));
//}
//
//TEST_F(TestSQLExtendedFetch, SingleCol_MultiRow_MultiFetch_Misaligned) {
//    EXPECT_NO_THROW(QueryBindExtendedFetch(multi_row_cnt, single_col_cnt,
//                                           multi_row_rd_cnt_misaligned,
//                                           single_col, &m_hstmt));
//}
//
//TEST_F(TestSQLExtendedFetch, MultiCol_SingleRow) {
//    EXPECT_NO_THROW(QueryBindExtendedFetch(
//        single_row_cnt, multi_col_cnt, single_row_rd_cnt, multi_col, &m_hstmt));
//}
//
//TEST_F(TestSQLExtendedFetch, MultiCol_MultiRow_SingleFetch) {
//    EXPECT_NO_THROW(QueryBindExtendedFetch(
//        multi_row_cnt, multi_col_cnt, single_row_rd_cnt, multi_col, &m_hstmt));
//}
//
//TEST_F(TestSQLExtendedFetch, MultiCol_MultiRow_MultiFetch_Aligned) {
//    EXPECT_NO_THROW(QueryBindExtendedFetch(multi_row_cnt, multi_col_cnt,
//                                           multi_row_rd_cnt_aligned, multi_col,
//                                           &m_hstmt));
//}
//
//TEST_F(TestSQLExtendedFetch, MultiCol_MultiRow_MultiFetch_Misaligned) {
//    EXPECT_NO_THROW(QueryBindExtendedFetch(multi_row_cnt, multi_col_cnt,
//                                           multi_row_rd_cnt_misaligned,
//                                           multi_col, &m_hstmt));
//}

//TEST_F(TestSQLGetData, GetWVARCHARData) {
//    QueryFetch(single_col, flight_data_set, single_row, &m_hstmt);
//
//    SQLRETURN ret =
//        SQLGetData(m_hstmt, m_single_column_ordinal_position, SQL_C_WCHAR,
//                   m_origin_column_data, m_origin_column_buffer_length,
//                   &m_origin_indicator);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//    EXPECT_TRUE(SQL_SUCCEEDED(ret));
//    // TODO (#110): Improve sample data result checks
//    bool found_expected_data =
//        wcscmp(m_origin_column_data, m_expected_origin_column_data_1.c_str())
//        || wcscmp(m_origin_column_data,
//                  m_expected_origin_column_data_2.c_str());
//    EXPECT_TRUE(found_expected_data);
//}
//
//TEST_F(TestSQLGetData, GetFloatData) {
//    QueryFetch(single_float_col, flight_data_set, single_row, &m_hstmt);
//
//    float data = 0.0f;
//    SQLRETURN ret = SQLGetData(m_hstmt, m_single_column_ordinal_position,
//                               SQL_C_FLOAT, &data, 4, &m_origin_indicator);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//    EXPECT_TRUE(SQL_SUCCEEDED(ret));
//    // TODO (#110): Improve sample data result checks
//    printf("%f\n", data);
//    bool found_expected_data =
//        (data == distance_miles_1 || data == distance_miles_2);
//    EXPECT_TRUE(found_expected_data);
//}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_STINYINT) {
    int v1 = -293719;
    int v2 = 741370;
    std::wstring columns = L"INTEGER\'" + std::to_wstring(v1) + L"\', INTEGER\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v1 = static_cast< SQLSCHAR >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v2 = static_cast< SQLSCHAR >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_TINYINT) {
    int v1 = -293719;
    int v2 = 741370;
    std::wstring columns = L"INTEGER\'" + std::to_wstring(v1) + L"\', INTEGER\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v1 = static_cast< SQLSCHAR >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v2 = static_cast< SQLSCHAR >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_UTINYINT) {
    int v1 = -293719;
    int v2 = 741370;
    std::wstring columns = L"INTEGER\'" + std::to_wstring(v1) + L"\', INTEGER\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v1 = static_cast< SQLCHAR >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v2 = static_cast< SQLCHAR >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_SLONG) {
    int v1 = -293719;
    int v2 = 741370;
    std::wstring columns = L"INTEGER\'" + std::to_wstring(v1) + L"\', INTEGER\'" + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SLONG , &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v1 = static_cast< SQLINTEGER >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SLONG , &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v2 = static_cast< SQLINTEGER >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_LONG) {
    int v1 = -293719;
    int v2 = 741370;
    std::wstring columns = L"INTEGER\'" + std::to_wstring(v1) + L"\', INTEGER\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v1 = static_cast< SQLINTEGER >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v2 = static_cast< SQLINTEGER >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_ULONG) {
    int v1 = -293719;
    int v2 = 741370;
    std::wstring columns = L"INTEGER\'" + std::to_wstring(v1) + L"\', INTEGER\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUINTEGER expected_v1 = static_cast< SQLUINTEGER >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUINTEGER expected_v2 = static_cast< SQLUINTEGER >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_SSHORT) {
    int v1 = -293719;
    int v2 = 741370;
    std::wstring columns = L"INTEGER\'" + std::to_wstring(v1) + L"\', INTEGER\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v1 = static_cast< SQLSMALLINT >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v2 = static_cast< SQLSMALLINT >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_SHORT) {
    int v1 = -293719;
    int v2 = 741370;
    std::wstring columns = L"INTEGER\'" + std::to_wstring(v1) + L"\', INTEGER\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v1 = static_cast< SQLSMALLINT >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v2 = static_cast< SQLSMALLINT >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_USHORT) {
    int v1 = -293719;
    int v2 = 741370;
    std::wstring columns = L"INTEGER\'" + std::to_wstring(v1) + L"\', INTEGER\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUSMALLINT expected_v1 = static_cast< SQLUSMALLINT >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUSMALLINT expected_v2 = static_cast< SQLUSMALLINT >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_SBIGINT) {
    int v1 = -293719;
    int v2 = 741370;
    std::wstring columns = L"INTEGER\'" + std::to_wstring(v1) + L"\', INTEGER\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLBIGINT expected_v1 = static_cast< SQLBIGINT >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLBIGINT expected_v2 = static_cast< SQLBIGINT >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_UBIGINT) {
    int v1 = -293719;
    int v2 = 741370;
    std::wstring columns = L"INTEGER\'" + std::to_wstring(v1) + L"\', INTEGER\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUBIGINT expected_v1 = static_cast< SQLUBIGINT >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUBIGINT expected_v2 = static_cast< SQLUBIGINT >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, INTEGER_TO_SQL_C_CHAR) {
    int v1 = -293719;
    int v2 = 741370;
    std::wstring columns = L"INTEGER\'" + std::to_wstring(v1) + L"\', INTEGER\'"
                           + std::to_wstring(v2) + L"\'";
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

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_STINYINT) {
    double v1 = 1.1;
    double v2 = -3.9E-5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v1 = static_cast< SQLSCHAR >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_STINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v2 = static_cast< SQLSCHAR >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_TINYINT) {
    double v1 = 1.1;
    double v2 = -3.9E5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v1 = static_cast< SQLSCHAR >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_TINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSCHAR expected_v2 =
        static_cast< SQLSCHAR >(strtol(std::to_string(v2).c_str(), NULL, 10));
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_UTINYINT) {
    double v1 = 1.1;
    double v2 = -3.0;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v1 = static_cast< SQLCHAR >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_UTINYINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR expected_v2 =
        static_cast< SQLCHAR >(strtol(std::to_string(v2).c_str(), NULL, 10));
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_SLONG) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v1 = static_cast< SQLINTEGER >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v2 = static_cast< SQLINTEGER >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_LONG) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v1 = static_cast< SQLINTEGER >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_LONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLINTEGER expected_v2 = static_cast< SQLINTEGER >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_ULONG) {
    double v1 = -293719.0;
    double v2 = 7.41370E5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUINTEGER expected_v1 = static_cast< SQLUINTEGER >(
        strtoul(std::to_string(v1).c_str(), NULL, 10));
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUINTEGER expected_v2 = static_cast< SQLUINTEGER >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_SSHORT) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v1 = static_cast< SQLSMALLINT >(
        strtol(std::to_string(v1).c_str(), NULL, 10));
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SSHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v2 = static_cast< SQLSMALLINT >(
        strtol(std::to_string(v2).c_str(), NULL, 10));
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_SHORT) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v1 = static_cast< SQLSMALLINT >(
        strtol(std::to_string(v1).c_str(), NULL, 10));
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT expected_v2 = static_cast< SQLSMALLINT >(
        strtol(std::to_string(v2).c_str(), NULL, 10));
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_USHORT) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUSMALLINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUSMALLINT expected_v1 = static_cast< SQLUSMALLINT >(
        strtol(std::to_string(v1).c_str(), NULL, 10));
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_USHORT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUSMALLINT expected_v2 = static_cast< SQLUSMALLINT >(
        strtol(std::to_string(v2).c_str(), NULL, 10));
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_SBIGINT) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLBIGINT expected_v1 = static_cast< SQLBIGINT >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLBIGINT expected_v2 = static_cast< SQLBIGINT >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_UBIGINT) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUBIGINT expected_v1 = static_cast< SQLUBIGINT >(
        strtoull(std::to_string(v1).c_str(), NULL, 10));
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLUBIGINT expected_v2 = static_cast< SQLUBIGINT >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_CHAR) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
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

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_DOUBLE) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLDOUBLE data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_DOUBLE, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_DOUBLE, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, DOUBLE_TO_SQL_C_FLOAT) {
    double v1 = -2.93719E5;
    double v2 = 7.41370E5;
    std::wstring columns = L"DOUBLE\'" + std::to_wstring(v1) + L"\', DOUBLE\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLREAL data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_FLOAT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLREAL expected_v1 = static_cast< SQLREAL >(v1);
    EXPECT_EQ(expected_v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_FLOAT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLREAL expected_v2 = static_cast< SQLREAL >(v2);
    EXPECT_EQ(expected_v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BIGINT_TO_SQL_C_SBIGINT) {
    auto v1 = 2147483649ll;
    auto v2 = 2147483649ll;
    v2 *= -1;
    std::wstring columns = L"BIGINT\'" + std::to_wstring(v1) + L"\', BIGINT\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(v1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(v2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BIGINT_TO_SQL_C_UBIGINT) {
    auto v1 = 2147483649ll;
    unsigned long long uv1 = v1;
    auto v2 = 2147483649ll;
    v2 *= -1;
    unsigned long long uv2 = v2;
    std::wstring columns = L"BIGINT\'" + std::to_wstring(v1) + L"\', BIGINT\'"
                           + std::to_wstring(v2) + L"\'";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUBIGINT data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(uv1, data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_UBIGINT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(uv2, data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BIGINT_TO_SQL_C_CHAR) {
    auto v1 = 2147483649ll;
    auto v2 = 2147483649ll;
    v2 *= -1;
    std::wstring columns = L"BIGINT\'" + std::to_wstring(v1) + L"\', BIGINT\'"
                           + std::to_wstring(v2) + L"\'";
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
    std::wstring columns = L"true, false";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    bool data = false;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_TRUE(data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_FALSE(data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BOOLEAN_TO_SQL_C_SLONG) {
    std::wstring columns = L"true, false";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_TRUE(data != 0);
    ret = SQLGetData(m_hstmt, 2, SQL_C_SLONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_TRUE(data == 0);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BOOLEAN_TO_SQL_C_ULONG) {
    std::wstring columns = L"true, false";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLUINTEGER data = 0;
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_TRUE(data != 0);
    ret = SQLGetData(m_hstmt, 2, SQL_C_ULONG, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_TRUE(data == 0);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, BOOLEAN_TO_SQL_C_CHAR) {
    std::wstring columns = L"true, false";
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
    std::wstring statement =
        L"WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 "
        L"14:18:30.123456789' AS binned_timestamp, ARRAY[1,2,3] "
        L"AS data FROM ODBCTest.IoT LIMIT "
        L"1), interpolated_timeseries AS(SELECT "
        L"CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) "
        L"SELECT *FROM interpolated_timeseries";
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
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected;
    expected = "[{time: 2021-03-05 14:18:30.123456789, value: [1, 2, 3]}]";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESERIES_TO_SQL_C_WCHAR_ARRAY) {
    SQLRETURN ret = SQL_ERROR;
    std::wstring statement =
        L"WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 "
        L"14:18:30.123456789' AS binned_timestamp, ARRAY[1,2,3] "
        L"AS data FROM ODBCTest.IoT LIMIT "
        L"1), interpolated_timeseries AS(SELECT "
        L"CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) "
        L"SELECT *FROM interpolated_timeseries";
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
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::wstring expected;
    expected = L"[{time: 2021-03-05 14:18:30.123456789, value: [1, 2, 3]}]";
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected.size()), indicator);
#else
    ASSERT_EQ((int)(2 * expected.size()), indicator);
#endif
    EXPECT_STREQ(expected.c_str(), (wchar_t*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESERIES_TO_SQL_C_CHAR_ROW) {
    SQLRETURN ret = SQL_ERROR;
    std::wstring statement =
        L"WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 "
        L"14:18:30.123456789' AS binned_timestamp, CAST(ROW(9.9, 19) AS "
        L"ROW(sum DOUBLE, count INTEGER)) AS data FROM ODBCTest.IoT LIMIT "
        L"1), interpolated_timeseries AS(SELECT "
        L"CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) "
        L"SELECT *FROM interpolated_timeseries";
    ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)statement.c_str(), (SQLINTEGER)statement.length());
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    ret = SQLFetch(m_hstmt);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected;
    expected = "[{time: 2021-03-05 14:18:30.123456789, value: (9.9, 19)}]";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESERIES_TO_SQL_C_WCHAR_ROW) {
    SQLRETURN ret = SQL_ERROR;
    std::wstring statement =
        L"WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 "
        L"14:18:30.123456789' AS binned_timestamp, CAST(ROW(9.9, 19) AS "
        L"ROW(sum DOUBLE, count INTEGER)) AS data FROM ODBCTest.IoT LIMIT "
        L"1), interpolated_timeseries AS(SELECT "
        L"CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) "
        L"SELECT *FROM interpolated_timeseries";
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
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::wstring expected;
    expected = L"[{time: 2021-03-05 14:18:30.123456789, value: (9.9, 19)}]";
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected.size()), indicator);
#else
    ASSERT_EQ((int)(2 * expected.size()), indicator);
#endif
    EXPECT_STREQ(expected.c_str(), (wchar_t*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESERIES_TO_SQL_C_CHAR_ARRAY_ROW_COMBINATION) {
    SQLRETURN ret = SQL_ERROR;
    std::wstring statement =
        L"WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 "
        L"14:18:30.123456789' AS binned_timestamp, ROW(null, ARRAY[ARRAY[ROW(12345, ARRAY[1, 2, 3])]]) "
        L"AS data FROM ODBCTest.IoT LIMIT "
        L"1), interpolated_timeseries AS(SELECT "
        L"CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) "
        L"SELECT *FROM interpolated_timeseries";
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
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected;
    expected = "[{time: 2021-03-05 14:18:30.123456789, value: (null, [[(12345, [1, 2, 3])]])}]";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, TIMESERIES_TO_SQL_C_WCHAR_ARRAY_ROW_COMBINATION) {
    SQLRETURN ret = SQL_ERROR;
    std::wstring statement =
        L"WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 "
        L"14:18:30.123456789' AS binned_timestamp, ROW(null, ARRAY[ARRAY[ROW(12345, ARRAY[1, 2, 3])]]) "
        L"AS data FROM ODBCTest.IoT LIMIT "
        L"1), interpolated_timeseries AS(SELECT "
        L"CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) "
        L"SELECT *FROM interpolated_timeseries";
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
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::wstring expected;
    expected = L"[{time: 2021-03-05 14:18:30.123456789, value: (null, [[(12345, [1, 2, 3])]])}]";
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected.size()), indicator);
#else
    ASSERT_EQ((int)(2 * expected.size()), indicator);
#endif
    EXPECT_STREQ(expected.c_str(), (wchar_t*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, ARRAY_TO_SQL_C_CHAR) {
    std::wstring columns = L"ARRAY[ARRAY[ARRAY[ARRAY[1.1, 2.3], ARRAY[1.1, 2.3]]], ARRAY[ARRAY[ARRAY[1.1, 2.3], ARRAY[1.1, 2.3]]]], ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[1, 2, 3]]]]]]]]]]]], ARRAY[]";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected;
    expected = "[[[[1.1, 2.3], [1.1, 2.3]]], [[[1.1, 2.3], [1.1, 2.3]]]]";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "[[[[[[[[[[[[1, 2, 3]]]]]]]]]]]]";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "-";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, ARRAY_TO_SQL_C_WCHAR) {
    std::wstring columns =
        L"ARRAY[ARRAY[ARRAY[ARRAY[1.1, 2.3], ARRAY[1.1, 2.3]]], "
        L"ARRAY[ARRAY[ARRAY[1.1, 2.3], ARRAY[1.1, 2.3]]]], "
        L"ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[ARRAY[1, 2, 3]]]]]]]]]]]], "
        L"ARRAY[]";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::wstring expected;
    expected = L"[[[[1.1, 2.3], [1.1, 2.3]]], [[[1.1, 2.3], [1.1, 2.3]]]]";
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected.size()), indicator);
#else
    ASSERT_EQ((int)(2 * expected.size()), indicator);
#endif
    EXPECT_STREQ(expected.c_str(), (wchar_t*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = L"[[[[[[[[[[[[1, 2, 3]]]]]]]]]]]]";
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected.size()), indicator);
#else
    ASSERT_EQ((int)(2 * expected.size()), indicator);
#endif
    EXPECT_STREQ(expected.c_str(), (wchar_t*)data);
    ret = SQLGetData(m_hstmt, 3, SQL_C_WCHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = L"-";
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected.size()), indicator);
#else
    ASSERT_EQ((int)(2 * expected.size()), indicator);
#endif
    EXPECT_STREQ(expected.c_str(), (wchar_t*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, ROW_TO_SQL_C_CHAR) {
    std::wstring columns =
        L"ROW(ROW(ROW(INTEGER '03', BIGINT '10', true), "
        L"ARRAY[ARRAY[1,2],ARRAY[1.1,2.2]])), ROW(true)";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected;
    expected = "(((3, 10, true), [[1.0, 2.0], [1.1, 2.2]]))";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "(true)";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, ROW_TO_SQL_C_WCHAR) {
    std::wstring columns =
        L"ROW(ROW(ROW(INTEGER '03', BIGINT '10', true), "
        L"ARRAY[ARRAY[1,2],ARRAY[1.1,2.2]])), ROW(true)";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::wstring expected;
    expected = L"(((3, 10, true), [[1.0, 2.0], [1.1, 2.2]]))";
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected.size()), indicator);
#else
    ASSERT_EQ((int)(2 * expected.size()), indicator);
#endif
    EXPECT_STREQ(expected.c_str(), (wchar_t*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = L"(true)";
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected.size()), indicator);
#else
    ASSERT_EQ((int)(2 * expected.size()), indicator);
#endif
    EXPECT_STREQ(expected.c_str(), (wchar_t*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, NULL_TO_SQL_C_CHAR) {
    std::wstring columns =
        L"null, NULL";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected;
    expected = "null";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "null";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, NULL_TO_SQL_C_WCHAR) {
    std::wstring columns = L"null, NULL";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::wstring expected;
    expected = L"null";
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected.size()), indicator);
#else
    ASSERT_EQ((int)(2 * expected.size()), indicator);
#endif
    EXPECT_STREQ(expected.c_str(), (wchar_t*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = L"null";
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected.size()), indicator);
#else
    ASSERT_EQ((int)(2 * expected.size()), indicator);
#endif
    EXPECT_STREQ(expected.c_str(), (wchar_t*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, ARRAY_ROW_NULL_TO_SQL_C_CHAR) {
    std::wstring columns = L"Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL])";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected;
    expected = "[(null), (null)]";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_CHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = "([null], [null])";
    ASSERT_EQ((int)expected.size(), indicator);
    EXPECT_STREQ(expected.c_str(), (char*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetData, ARRAY_ROW_NULL_TO_SQL_C_WCHAR) {
    std::wstring columns =
        L"Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL])";
    QueryFetch(columns, table_name, single_row, &m_hstmt);
    SQLTCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetData(m_hstmt, 1, SQL_C_WCHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::wstring expected;
    expected = L"[(null), (null)]";
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected.size()), indicator);
#else
    ASSERT_EQ((int)(2 * expected.size()), indicator);
#endif
    EXPECT_STREQ(expected.c_str(), (wchar_t*)data);
    ret = SQLGetData(m_hstmt, 2, SQL_C_WCHAR, data, 1024, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    expected = L"([null], [null])";
#ifdef __APPLE__
    ASSERT_EQ((int)(4 * expected.size()), indicator);
#else
    ASSERT_EQ((int)(2 * expected.size()), indicator);
#endif
    EXPECT_STREQ(expected.c_str(), (wchar_t*)data);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}



//TEST_F(TestSQLGetData, GetBitData) {
//    QueryFetch(single_bit_col, flight_data_set, single_row, &m_hstmt);
//
//    bool data_false;
//    SQLRETURN ret = SQLGetData(m_hstmt, m_single_column_ordinal_position,
//                               SQL_C_BIT, &data_false, 1, &m_origin_indicator);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//    EXPECT_TRUE(SQL_SUCCEEDED(ret));
//    // TODO (#110): Improve sample data result checks
//    // Since it can return either true or false, will disable check for now
//    // EXPECT_FALSE(data_false);
//
//    // Send another query
//    ASSERT_NO_THROW(CloseCursor(&m_hstmt, true, true));
//    QueryFetch(single_bit_col, flight_data_set, single_row_offset_3, &m_hstmt);
//
//    ret = SQLGetData(m_hstmt, m_single_column_ordinal_position, SQL_C_BIT,
//                     &data_false, 1, &m_origin_indicator);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//    EXPECT_TRUE(SQL_SUCCEEDED(ret));
//    // TODO (#110): Improve sample data result checks
//    // Since it can return either true or false, will disable check for now
//    // EXPECT_FALSE(data_false);
//}

//GET_DATA_TEST(TypeDataSet_GetBoolData, type_boolean, SQL_C_BIT,
//              type_boolean_vals, 1)
//
//GET_DATA_TEST(TypeDataSet_GetByteData, type_byte, SQL_C_SHORT, type_byte_vals,
//              2)
//
//GET_DATA_TEST(TypeDataSet_GetDateData, type_date, SQL_C_TIMESTAMP,
//              type_date_vals, sizeof(TIMESTAMP_STRUCT))
//
//GET_DATA_TEST(TypeDataSet_GetShortData, type_short, SQL_C_SHORT,
//              type_short_vals, 2)
//
//GET_DATA_TEST(TypeDataSet_GetIntegerData, type_integer, SQL_C_LONG,
//              type_integer_vals, 4)
//
//GET_DATA_TEST(TypeDataSet_GetLongData, type_long, SQL_C_SBIGINT, type_long_vals,
//              8)
//
//GET_DATA_TEST(TypeDataSet_GetHalfFloatData, type_half_float, SQL_C_FLOAT,
//              type_half_float_vals, 4)
//
//GET_DATA_TEST(TypeDataSet_GetFloatData, type_float, SQL_C_FLOAT,
//              type_float_vals, 4)
//
//GET_DATA_TEST(TypeDataSet_GetDoubleData, type_double, SQL_C_DOUBLE,
//              type_double_vals, 8)
//
//GET_DATA_TEST(TypeDataSet_GetScaledFloatData, type_scaled_float, SQL_C_DOUBLE,
//              type_scaled_float_vals, 8)
//
//GET_DATA_TEST(TypeDataSet_GetKeywordData, type_keyword, SQL_C_WCHAR,
//              type_keyword_vals, 512)
//GET_DATA_TEST(TypeDataSet_GetKeywordDataMultiReadSingleByte, type_keyword,
//              SQL_C_WCHAR, type_keyword_vals, 2)
//GET_DATA_TEST(TypeDataSet_GetKeywordDataMultiReadMultiByte, type_keyword,
//              SQL_C_WCHAR, type_keyword_vals, 4)
//
//GET_DATA_TEST(TypeDataSet_GetTextData, type_text, SQL_C_WCHAR, type_text_vals,
//              512)
//GET_DATA_TEST(TypeDataSet_GetTextDataMultiReadSingleByte, type_text,
//              SQL_C_WCHAR, type_text_vals, 2)
//GET_DATA_TEST(TypeDataSet_GetTextDataMultiReadMultiByte, type_text, SQL_C_WCHAR,
//              type_text_vals, 4)
//
//TEST_F(TestSQLGetData, SQLSTATE_01004_StringDataRightTruncated) {
//    SQLRETURN ret;
//    SQLLEN insufficient_buffer_length = 1;
//
//    ExecuteQuery(single_col, flight_data_set, single_row, &m_hstmt);
//
//    ret = SQLFetch(m_hstmt);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//    ASSERT_TRUE(SQL_SUCCEEDED(ret));
//
//    ret = SQLGetData(m_hstmt, m_single_column_ordinal_position, SQL_C_WCHAR,
//                     m_origin_column_data, insufficient_buffer_length,
//                     &m_origin_indicator);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//    EXPECT_EQ(ret, SQL_SUCCESS_WITH_INFO);
//    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
//                              SQLSTATE_STRING_DATA_RIGHT_TRUNCATED));
//}
//
//TEST_F(TestSQLGetData, SQLSTATE_07009_InvalidDescriptorIndex) {
//    SQLRETURN ret;
//    SQLUSMALLINT invalid_column_ordinal_position = 2;
//
//    ExecuteQuery(single_col, flight_data_set, single_row, &m_hstmt);
//
//    ret = SQLFetch(m_hstmt);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//    ASSERT_TRUE(SQL_SUCCEEDED(ret));
//
//    ret = SQLGetData(m_hstmt, invalid_column_ordinal_position, SQL_C_WCHAR,
//                     m_origin_column_data, m_origin_column_buffer_length,
//                     &m_origin_indicator);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//    EXPECT_EQ(ret, SQL_ERROR);
//    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
//                              SQLSTATE_INVALID_DESCRIPTOR_INDEX));
//}

TEST_F(TestSQLNumResultCols, SingleColumn) {
    std::wstring columns =
        L"Array[Row(null), Row(NULL)]";
    ExecuteQuery(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT column_count = 0;
    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(m_hstmt, &column_count));
    EXPECT_EQ(1, column_count);
}

TEST_F(TestSQLNumResultCols, MultiColumn) {
    std::wstring columns =
        L"Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL]), "
        L"Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL]), "
        L"Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL]), "
        L"Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL]), "
        L"Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL]), "
        L"Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL]), "
        L"Array[Row(null), Row(NULL)], Row(Array[null], Array[NULL])";
    ExecuteQuery(columns, table_name, single_row, &m_hstmt);
    SQLSMALLINT column_count = 0;
    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(m_hstmt, &column_count));
    EXPECT_EQ(14, column_count);
}
//
//TEST_F(TestSQLDescribeCol, SingleColumnMetadata) {
//    ExecuteQuery(single_col, flight_data_set, single_row, &m_hstmt);
//
//    EXPECT_EQ(
//        SQL_SUCCESS,
//        SQLDescribeCol(m_hstmt, 1, m_column_name, IT_SIZEOF(m_column_name),
//                       &m_column_name_length, &m_data_type, &m_column_size,
//                       &m_decimal_digits, &m_nullable));
//    EXPECT_EQ(single_col, m_column_name);
//    EXPECT_EQ(single_col_name_length, m_column_name_length);
//    EXPECT_EQ(single_col_data_type, m_data_type);
//    // TODO #628 - Investigate why value differs & fix validation accordingly
//    EXPECT_EQ(single_col_column_size, m_column_size);
//    EXPECT_EQ(single_col_decimal_digit, m_decimal_digits);
//    EXPECT_EQ(single_col_nullable, m_nullable);
//}
//
//TEST_F(TestSQLDescribeCol, MultiColumnMetadata) {
//    ExecuteQuery(multi_col, flight_data_set, multi_row, &m_hstmt);
//
//    for (SQLUSMALLINT i = 1; i <= multi_col_cnt; i++) {
//        EXPECT_EQ(
//            SQL_SUCCESS,
//            SQLDescribeCol(m_hstmt, i, m_column_name, IT_SIZEOF(m_column_name),
//                           &m_column_name_length, &m_data_type, &m_column_size,
//                           &m_decimal_digits, &m_nullable));
//    }
//}
//
//TEST_F(TestSQLDescribeCol, MultiColumnNameLengthType) {
//    ExecuteQuery(multi_col, flight_data_set, multi_row, &m_hstmt);
//
//    for (SQLUSMALLINT i = 1; i <= multi_col_cnt; i++) {
//        EXPECT_EQ(
//            SQL_SUCCESS,
//            SQLDescribeCol(m_hstmt, i, m_column_name, IT_SIZEOF(m_column_name),
//                           &m_column_name_length, &m_data_type, &m_column_size,
//                           &m_decimal_digits, &m_nullable));
//        EXPECT_EQ(column_data[i - 1].column_name, std::wstring(m_column_name));
//        EXPECT_EQ(column_data[i - 1].data_type, m_data_type);
//    }
//}
//
//TEST_F(TestSQLDescribeCol, InvalidColumnMetadata) {
//    ExecuteQuery(multi_col, flight_data_set, multi_row, &m_hstmt);
//
//    EXPECT_EQ(SQL_ERROR,
//              SQLDescribeCol(m_hstmt, multi_col_cnt + 1, m_column_name,
//                             IT_SIZEOF(m_column_name), &m_column_name_length,
//                             &m_data_type, &m_column_size, &m_decimal_digits,
//                             &m_nullable));
//    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
//                              SQLSTATE_INVALID_DESCRIPTOR_INDEX));
//}
//
//TEST_F(TestSQLMoreResults, NoData) {
//    SQLRETURN ret = SQLMoreResults(m_hstmt);
//    EXPECT_EQ(SQL_NO_DATA, ret);
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//}
//
//// Row count is not supported for the driver, so this should return -1,
//// as defined in the ODBC API.
//TEST_F(TestSQLRowCount, RowCountNotAvailable) {
//    SQLLEN row_count;
//    SQLRETURN ret = SQLRowCount(m_hstmt, &row_count);
//    EXPECT_EQ(SQL_SUCCESS, ret);
//    EXPECT_EQ(row_count, -1L);
//}

int main(int argc, char** argv) {
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
    system("leaks itodbc_results > leaks_itodbc_results");
#endif
    return failures;
}
