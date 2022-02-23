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

#ifdef _WIN32
#include <windows.h>
#endif

#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

#include "ignite/ignition.h"
#include "ignite/impl/binary/binary_utils.h"
#include "ignite/odbc/config/connection_info.h"
#include "ignite/odbc/system/odbc_constants.h"
#include "odbc_test_suite.h"
#include "test_type.h"
#include "test_utils.h"

using namespace ignite;
using namespace ignite_test;

using namespace boost::unit_test;

using ignite::impl::binary::BinaryUtils;

/**
 * Test setup fixture.
 */
struct SqlGetInfoTestSuiteFixture : odbc::OdbcTestSuite {
  void CheckStrInfo(SQLSMALLINT type, const std::string& expectedValue) {
    SQLCHAR val[ODBC_BUFFER_SIZE];
    SQLSMALLINT valLen = 0;

    SQLRETURN ret = SQLGetInfo(dbc, type, &val, sizeof(val), &valLen);

    std::string typeStr = odbc::config::ConnectionInfo::InfoTypeToString(type);
    ODBC_FAIL_ON_ERROR1(ret, SQL_HANDLE_DBC, dbc, typeStr);
    BOOST_CHECK_EQUAL(std::string(reinterpret_cast< const char* >(val), valLen),
                      expectedValue);
  }

  void CheckIntInfo(SQLSMALLINT type, SQLUINTEGER expectedValue) {
    SQLUINTEGER val;
    SQLRETURN ret = SQLGetInfo(dbc, type, &val, 0, 0);

    std::string typeStr = odbc::config::ConnectionInfo::InfoTypeToString(type);
    ODBC_FAIL_ON_ERROR1(ret, SQL_HANDLE_DBC, dbc, typeStr);
    BOOST_CHECK_EQUAL(val, expectedValue);
  }

  void CheckShortInfo(SQLSMALLINT type, SQLUSMALLINT expectedValue) {
    SQLUSMALLINT val;
    SQLRETURN ret = SQLGetInfo(dbc, type, &val, 0, 0);

    std::string typeStr = odbc::config::ConnectionInfo::InfoTypeToString(type);
    ODBC_FAIL_ON_ERROR1(ret, SQL_HANDLE_DBC, dbc, typeStr);
    BOOST_CHECK_EQUAL(val, expectedValue);
  }

  /**
   * Constructor.
   */
  SqlGetInfoTestSuiteFixture() {
    grid = StartPlatformNode("queries-test.xml", "NodeMain");
  }

  /**
   * Destructor.
   */
  ~SqlGetInfoTestSuiteFixture() {
    Ignition::StopAll(true);
  }

  /** Node started during the test. */
  Ignite grid;
};

BOOST_FIXTURE_TEST_SUITE(SqlGetInfoTestSuite, SqlGetInfoTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestValues) {
  Connect("DRIVER={Apache Ignite};address=127.0.0.1:11110;schema=cache");

  CheckStrInfo(SQL_DRIVER_NAME, "Apache Ignite");
  CheckStrInfo(SQL_DBMS_NAME, "Apache Ignite");
  CheckStrInfo(SQL_DRIVER_ODBC_VER, "03.00");
  CheckStrInfo(SQL_DRIVER_VER, "02.04.0000");
  CheckStrInfo(SQL_DBMS_VER, "02.04.0000");
  CheckStrInfo(SQL_COLUMN_ALIAS, "Y");
  CheckStrInfo(SQL_IDENTIFIER_QUOTE_CHAR, "");
  CheckStrInfo(SQL_CATALOG_NAME_SEPARATOR, ".");
  CheckStrInfo(SQL_SPECIAL_CHARACTERS, "");
  CheckStrInfo(SQL_CATALOG_TERM, "");
  CheckStrInfo(SQL_QUALIFIER_TERM, "");
  CheckStrInfo(SQL_TABLE_TERM, "table");
  CheckStrInfo(SQL_SCHEMA_TERM, "schema");
  CheckStrInfo(SQL_NEED_LONG_DATA_LEN, "Y");
  CheckStrInfo(SQL_ACCESSIBLE_PROCEDURES, "Y");
  CheckStrInfo(SQL_ACCESSIBLE_TABLES, "Y");
  CheckStrInfo(SQL_CATALOG_NAME, "N");
  CheckStrInfo(SQL_COLLATION_SEQ, "UTF-8");
  CheckStrInfo(SQL_DATA_SOURCE_NAME, "");
  CheckStrInfo(SQL_DATA_SOURCE_READ_ONLY, "N");
  CheckStrInfo(SQL_DATABASE_NAME, "");
  CheckStrInfo(SQL_DESCRIBE_PARAMETER, "N");
  CheckStrInfo(SQL_EXPRESSIONS_IN_ORDERBY, "Y");
  CheckStrInfo(SQL_INTEGRITY, "N");
  CheckStrInfo(SQL_KEYWORDS,
               "LIMIT,MINUS,OFFSET,ROWNUM,SYSDATE,SYSTIME,SYSTIMESTAMP,TODAY");
  CheckStrInfo(SQL_LIKE_ESCAPE_CLAUSE, "N");
  CheckStrInfo(SQL_MAX_ROW_SIZE_INCLUDES_LONG, "Y");
  CheckStrInfo(SQL_MULT_RESULT_SETS, "N");
  CheckStrInfo(SQL_MULTIPLE_ACTIVE_TXN, "Y");
  CheckStrInfo(SQL_ORDER_BY_COLUMNS_IN_SELECT, "N");
  CheckStrInfo(SQL_PROCEDURE_TERM, "stored procedure");
  CheckStrInfo(SQL_PROCEDURES, "N");
  CheckStrInfo(SQL_ROW_UPDATES, "N");
  CheckStrInfo(SQL_SEARCH_PATTERN_ESCAPE, "\\");
  CheckStrInfo(SQL_SERVER_NAME, "Apache Ignite");
  CheckStrInfo(SQL_USER_NAME, "apache_ignite_user");

  CheckIntInfo(SQL_ASYNC_MODE, SQL_AM_NONE);
  CheckIntInfo(SQL_BATCH_ROW_COUNT, SQL_BRC_ROLLED_UP | SQL_BRC_EXPLICIT);
  CheckIntInfo(SQL_BATCH_SUPPORT, SQL_BS_ROW_COUNT_EXPLICIT);
  CheckIntInfo(SQL_BOOKMARK_PERSISTENCE, 0);
  CheckIntInfo(SQL_CATALOG_LOCATION, 0);
  CheckIntInfo(SQL_QUALIFIER_LOCATION, 0);
  CheckIntInfo(SQL_GETDATA_EXTENSIONS,
               SQL_GD_ANY_COLUMN | SQL_GD_ANY_ORDER | SQL_GD_BOUND);
  CheckIntInfo(SQL_ODBC_INTERFACE_CONFORMANCE, SQL_OIC_CORE);
  CheckIntInfo(SQL_SQL_CONFORMANCE, SQL_SC_SQL92_ENTRY);
  CheckIntInfo(SQL_CATALOG_USAGE, 0);
  CheckIntInfo(SQL_QUALIFIER_USAGE, 0);
  CheckIntInfo(SQL_TIMEDATE_ADD_INTERVALS, 0);
  CheckIntInfo(SQL_TIMEDATE_DIFF_INTERVALS, 0);
  CheckIntInfo(SQL_DATETIME_LITERALS,
               SQL_DL_SQL92_DATE | SQL_DL_SQL92_TIME | SQL_DL_SQL92_TIMESTAMP);
  CheckIntInfo(SQL_SYSTEM_FUNCTIONS,
               SQL_FN_SYS_USERNAME | SQL_FN_SYS_DBNAME | SQL_FN_SYS_IFNULL);
  CheckIntInfo(SQL_CONVERT_FUNCTIONS, SQL_FN_CVT_CONVERT | SQL_FN_CVT_CAST);
  CheckIntInfo(SQL_OJ_CAPABILITIES,
               SQL_OJ_LEFT | SQL_OJ_NOT_ORDERED | SQL_OJ_ALL_COMPARISON_OPS);
  CheckIntInfo(SQL_POS_OPERATIONS, 0);
  CheckIntInfo(SQL_SQL92_DATETIME_FUNCTIONS,
               SQL_SDF_CURRENT_DATE | SQL_SDF_CURRENT_TIMESTAMP);
  CheckIntInfo(SQL_SQL92_VALUE_EXPRESSIONS,
               SQL_SVE_CASE | SQL_SVE_CAST | SQL_SVE_COALESCE | SQL_SVE_NULLIF);
  CheckIntInfo(SQL_STATIC_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT | SQL_CA1_ABSOLUTE);
  CheckIntInfo(SQL_STATIC_CURSOR_ATTRIBUTES2, 0);
  CheckIntInfo(SQL_PARAM_ARRAY_ROW_COUNTS, SQL_PARC_BATCH);
  CheckIntInfo(SQL_PARAM_ARRAY_SELECTS, SQL_PAS_NO_SELECT);
  CheckIntInfo(SQL_SCROLL_OPTIONS, SQL_SO_FORWARD_ONLY | SQL_SO_STATIC);
  CheckIntInfo(SQL_ALTER_DOMAIN, 0);
  CheckIntInfo(SQL_ALTER_TABLE, SQL_AT_ADD_COLUMN_SINGLE);
  CheckIntInfo(SQL_CREATE_ASSERTION, 0);
  CheckIntInfo(SQL_CREATE_CHARACTER_SET, 0);
  CheckIntInfo(SQL_CREATE_COLLATION, 0);
  CheckIntInfo(SQL_CREATE_DOMAIN, 0);
  CheckIntInfo(SQL_CREATE_SCHEMA, 0);
  CheckIntInfo(SQL_CREATE_TABLE,
               SQL_CT_CREATE_TABLE | SQL_CT_COLUMN_CONSTRAINT);
  CheckIntInfo(SQL_CREATE_TRANSLATION, 0);
  CheckIntInfo(SQL_CREATE_VIEW, 0);
  CheckIntInfo(SQL_CURSOR_SENSITIVITY, SQL_INSENSITIVE);
  CheckIntInfo(SQL_DDL_INDEX, SQL_DI_CREATE_INDEX | SQL_DI_DROP_INDEX);
  CheckIntInfo(SQL_DEFAULT_TXN_ISOLATION, SQL_TXN_REPEATABLE_READ);
  CheckIntInfo(SQL_DROP_ASSERTION, 0);
  CheckIntInfo(SQL_DROP_CHARACTER_SET, 0);
  CheckIntInfo(SQL_DROP_COLLATION, 0);
  CheckIntInfo(SQL_DROP_DOMAIN, 0);
  CheckIntInfo(SQL_DROP_SCHEMA, 0);
  CheckIntInfo(SQL_DROP_TABLE, SQL_DT_DROP_TABLE);
  CheckIntInfo(SQL_DROP_TRANSLATION, 0);
  CheckIntInfo(SQL_DROP_VIEW, 0);
  CheckIntInfo(SQL_DYNAMIC_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT);
  CheckIntInfo(SQL_DYNAMIC_CURSOR_ATTRIBUTES2, SQL_CA2_READ_ONLY_CONCURRENCY);
  CheckIntInfo(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT);
  CheckIntInfo(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2,
               SQL_CA2_READ_ONLY_CONCURRENCY);
  CheckIntInfo(SQL_INDEX_KEYWORDS, SQL_IK_ALL);
  CheckIntInfo(SQL_INFO_SCHEMA_VIEWS, 0);
  CheckIntInfo(SQL_INSERT_STATEMENT,
               SQL_IS_INSERT_LITERALS | SQL_IS_INSERT_SEARCHED);
  CheckIntInfo(SQL_KEYSET_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT);
  CheckIntInfo(SQL_KEYSET_CURSOR_ATTRIBUTES2, 0);
  CheckIntInfo(SQL_MAX_ASYNC_CONCURRENT_STATEMENTS, 0);
  CheckIntInfo(SQL_MAX_BINARY_LITERAL_LEN, 0);
  CheckIntInfo(SQL_MAX_CATALOG_NAME_LEN, 0);
  CheckIntInfo(SQL_MAX_CHAR_LITERAL_LEN, 0);
  CheckIntInfo(SQL_MAX_INDEX_SIZE, 0);
  CheckIntInfo(SQL_MAX_ROW_SIZE, 0);
  CheckIntInfo(SQL_MAX_STATEMENT_LEN, 0);
  CheckIntInfo(SQL_SQL92_FOREIGN_KEY_DELETE_RULE, 0);
  CheckIntInfo(SQL_SQL92_FOREIGN_KEY_UPDATE_RULE, 0);
  CheckIntInfo(SQL_SQL92_GRANT, 0);
  CheckIntInfo(SQL_SQL92_REVOKE, 0);
  CheckIntInfo(SQL_STANDARD_CLI_CONFORMANCE, 0);
  CheckIntInfo(SQL_TXN_ISOLATION_OPTION, SQL_TXN_REPEATABLE_READ);
  CheckIntInfo(SQL_UNION, SQL_U_UNION | SQL_U_UNION_ALL);

  CheckIntInfo(SQL_SCHEMA_USAGE, SQL_SU_DML_STATEMENTS | SQL_SU_TABLE_DEFINITION
                                     | SQL_SU_PRIVILEGE_DEFINITION
                                     | SQL_SU_INDEX_DEFINITION);

  CheckIntInfo(SQL_AGGREGATE_FUNCTIONS, SQL_AF_AVG | SQL_AF_COUNT | SQL_AF_MAX
                                            | SQL_AF_MIN | SQL_AF_SUM
                                            | SQL_AF_DISTINCT);

  CheckIntInfo(SQL_NUMERIC_FUNCTIONS,
               SQL_FN_NUM_ABS | SQL_FN_NUM_ACOS | SQL_FN_NUM_ASIN
                   | SQL_FN_NUM_EXP | SQL_FN_NUM_ATAN | SQL_FN_NUM_ATAN2
                   | SQL_FN_NUM_CEILING | SQL_FN_NUM_COS | SQL_FN_NUM_TRUNCATE
                   | SQL_FN_NUM_FLOOR | SQL_FN_NUM_DEGREES | SQL_FN_NUM_POWER
                   | SQL_FN_NUM_RADIANS | SQL_FN_NUM_SIGN | SQL_FN_NUM_SIN
                   | SQL_FN_NUM_LOG | SQL_FN_NUM_TAN | SQL_FN_NUM_PI
                   | SQL_FN_NUM_MOD | SQL_FN_NUM_COT | SQL_FN_NUM_LOG10
                   | SQL_FN_NUM_ROUND | SQL_FN_NUM_SQRT | SQL_FN_NUM_RAND);

  CheckIntInfo(SQL_STRING_FUNCTIONS,
               SQL_FN_STR_ASCII | SQL_FN_STR_BIT_LENGTH | SQL_FN_STR_CHAR_LENGTH
                   | SQL_FN_STR_CHAR | SQL_FN_STR_CONCAT | SQL_FN_STR_DIFFERENCE
                   | SQL_FN_STR_INSERT | SQL_FN_STR_LEFT | SQL_FN_STR_LENGTH
                   | SQL_FN_STR_CHARACTER_LENGTH | SQL_FN_STR_LTRIM
                   | SQL_FN_STR_OCTET_LENGTH | SQL_FN_STR_POSITION
                   | SQL_FN_STR_REPEAT | SQL_FN_STR_REPLACE | SQL_FN_STR_RIGHT
                   | SQL_FN_STR_RTRIM | SQL_FN_STR_SOUNDEX | SQL_FN_STR_SPACE
                   | SQL_FN_STR_SUBSTRING | SQL_FN_STR_LCASE | SQL_FN_STR_UCASE
                   | SQL_FN_STR_LOCATE_2 | SQL_FN_STR_LOCATE);

  CheckIntInfo(SQL_TIMEDATE_FUNCTIONS,
               SQL_FN_TD_CURRENT_DATE | SQL_FN_TD_CURRENT_TIME | SQL_FN_TD_WEEK
                   | SQL_FN_TD_QUARTER | SQL_FN_TD_SECOND | SQL_FN_TD_CURDATE
                   | SQL_FN_TD_CURTIME | SQL_FN_TD_DAYNAME | SQL_FN_TD_MINUTE
                   | SQL_FN_TD_DAYOFWEEK | SQL_FN_TD_DAYOFYEAR
                   | SQL_FN_TD_EXTRACT | SQL_FN_TD_HOUR | SQL_FN_TD_DAYOFMONTH
                   | SQL_FN_TD_MONTH | SQL_FN_TD_MONTHNAME | SQL_FN_TD_NOW
                   | SQL_FN_TD_YEAR | SQL_FN_TD_CURRENT_TIMESTAMP);

  CheckIntInfo(SQL_SQL92_NUMERIC_VALUE_FUNCTIONS,
               SQL_SNVF_BIT_LENGTH | SQL_SNVF_CHARACTER_LENGTH
                   | SQL_SNVF_EXTRACT | SQL_SNVF_OCTET_LENGTH
                   | SQL_SNVF_POSITION);

  CheckIntInfo(SQL_SQL92_STRING_FUNCTIONS,
               SQL_SSF_LOWER | SQL_SSF_UPPER | SQL_SSF_TRIM_TRAILING
                   | SQL_SSF_SUBSTRING | SQL_SSF_TRIM_BOTH
                   | SQL_SSF_TRIM_LEADING);

  CheckIntInfo(SQL_SQL92_PREDICATES,
               SQL_SP_BETWEEN | SQL_SP_COMPARISON | SQL_SP_EXISTS | SQL_SP_IN
                   | SQL_SP_ISNOTNULL | SQL_SP_ISNULL | SQL_SP_LIKE
                   | SQL_SP_MATCH_FULL | SQL_SP_MATCH_PARTIAL
                   | SQL_SP_MATCH_UNIQUE_FULL | SQL_SP_MATCH_UNIQUE_PARTIAL
                   | SQL_SP_OVERLAPS | SQL_SP_UNIQUE
                   | SQL_SP_QUANTIFIED_COMPARISON);

  CheckIntInfo(SQL_SQL92_RELATIONAL_JOIN_OPERATORS,
               SQL_SRJO_CORRESPONDING_CLAUSE | SQL_SRJO_CROSS_JOIN
                   | SQL_SRJO_EXCEPT_JOIN | SQL_SRJO_INNER_JOIN
                   | SQL_SRJO_LEFT_OUTER_JOIN | SQL_SRJO_RIGHT_OUTER_JOIN
                   | SQL_SRJO_NATURAL_JOIN | SQL_SRJO_INTERSECT_JOIN
                   | SQL_SRJO_UNION_JOIN);

  CheckIntInfo(SQL_CONVERT_BIGINT,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
                   | SQL_CVT_NUMERIC | SQL_CVT_TIMESTAMP | SQL_CVT_TINYINT
                   | SQL_CVT_SMALLINT | SQL_CVT_INTEGER | SQL_CVT_BIGINT
                   | SQL_CVT_BIT);

  CheckIntInfo(SQL_CONVERT_BINARY,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_BIT | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR
                   | SQL_CVT_WVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_LONGVARBINARY
                   | SQL_CVT_FLOAT | SQL_CVT_SMALLINT | SQL_CVT_INTEGER
                   | SQL_CVT_BIGINT | SQL_CVT_REAL | SQL_CVT_DATE
                   | SQL_CVT_TINYINT | SQL_CVT_DOUBLE | SQL_CVT_BINARY
                   | SQL_CVT_DECIMAL | SQL_CVT_TIME | SQL_CVT_GUID
                   | SQL_CVT_TIMESTAMP | SQL_CVT_VARBINARY);

  CheckIntInfo(SQL_CONVERT_BIT, SQL_CVT_BIGINT | SQL_CVT_VARCHAR
                                    | SQL_CVT_LONGVARCHAR | SQL_CVT_WVARCHAR
                                    | SQL_CVT_NUMERIC | SQL_CVT_BIT
                                    | SQL_CVT_CHAR | SQL_CVT_SMALLINT
                                    | SQL_CVT_INTEGER | SQL_CVT_TINYINT);

  CheckIntInfo(
      SQL_CONVERT_CHAR,
      SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR | SQL_CVT_VARBINARY
          | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
          | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_BIT | SQL_CVT_TINYINT
          | SQL_CVT_SMALLINT | SQL_CVT_INTEGER | SQL_CVT_BIGINT | SQL_CVT_REAL
          | SQL_CVT_FLOAT | SQL_CVT_DOUBLE | SQL_CVT_BINARY | SQL_CVT_TIMESTAMP
          | SQL_CVT_DATE | SQL_CVT_TIME | SQL_CVT_LONGVARBINARY | SQL_CVT_GUID);

  CheckIntInfo(SQL_CONVERT_VARCHAR,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
                   | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_TINYINT
                   | SQL_CVT_SMALLINT | SQL_CVT_INTEGER | SQL_CVT_BIGINT
                   | SQL_CVT_REAL | SQL_CVT_FLOAT | SQL_CVT_BIT | SQL_CVT_DOUBLE
                   | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_GUID
                   | SQL_CVT_DATE | SQL_CVT_TIME | SQL_CVT_LONGVARBINARY
                   | SQL_CVT_TIMESTAMP);

  CheckIntInfo(SQL_CONVERT_LONGVARCHAR,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_DATE | SQL_CVT_TIME
                   | SQL_CVT_LONGVARCHAR | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR
                   | SQL_CVT_WVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL
                   | SQL_CVT_BIT | SQL_CVT_REAL | SQL_CVT_SMALLINT
                   | SQL_CVT_INTEGER | SQL_CVT_GUID | SQL_CVT_BIGINT
                   | SQL_CVT_LONGVARBINARY | SQL_CVT_DOUBLE | SQL_CVT_BINARY
                   | SQL_CVT_VARBINARY | SQL_CVT_TINYINT | SQL_CVT_FLOAT
                   | SQL_CVT_TIMESTAMP);

  CheckIntInfo(SQL_CONVERT_WCHAR,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
                   | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_VARBINARY
                   | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_SMALLINT
                   | SQL_CVT_INTEGER | SQL_CVT_BIGINT | SQL_CVT_REAL
                   | SQL_CVT_FLOAT | SQL_CVT_DOUBLE | SQL_CVT_BINARY
                   | SQL_CVT_LONGVARBINARY | SQL_CVT_TIMESTAMP | SQL_CVT_DATE
                   | SQL_CVT_TIME | SQL_CVT_GUID);

  CheckIntInfo(SQL_CONVERT_WVARCHAR,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_REAL | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR
                   | SQL_CVT_WVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_LONGVARBINARY
                   | SQL_CVT_DECIMAL | SQL_CVT_BIT | SQL_CVT_TINYINT
                   | SQL_CVT_SMALLINT | SQL_CVT_DATE | SQL_CVT_TIME
                   | SQL_CVT_INTEGER | SQL_CVT_BIGINT | SQL_CVT_FLOAT
                   | SQL_CVT_DOUBLE | SQL_CVT_BINARY | SQL_CVT_GUID
                   | SQL_CVT_VARBINARY | SQL_CVT_TIMESTAMP);

  CheckIntInfo(SQL_CONVERT_WLONGVARCHAR,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_BIGINT | SQL_CVT_REAL
                   | SQL_CVT_LONGVARCHAR | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR
                   | SQL_CVT_WVARCHAR | SQL_CVT_VARBINARY | SQL_CVT_NUMERIC
                   | SQL_CVT_DECIMAL | SQL_CVT_BIT | SQL_CVT_TINYINT
                   | SQL_CVT_DATE | SQL_CVT_FLOAT | SQL_CVT_INTEGER
                   | SQL_CVT_SMALLINT | SQL_CVT_DOUBLE | SQL_CVT_BINARY
                   | SQL_CVT_LONGVARBINARY | SQL_CVT_TIME | SQL_CVT_TIMESTAMP
                   | SQL_CVT_GUID);

  CheckIntInfo(SQL_CONVERT_GUID, SQL_CVT_CHAR | SQL_CVT_VARCHAR
                                     | SQL_CVT_LONGVARCHAR | SQL_CVT_WVARCHAR
                                     | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR
                                     | SQL_CVT_BINARY | SQL_CVT_VARBINARY
                                     | SQL_CVT_LONGVARBINARY | SQL_CVT_GUID);

  CheckIntInfo(SQL_CONVERT_DATE,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
                   | SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_BIGINT
                   | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_LONGVARBINARY
                   | SQL_CVT_DATE | SQL_CVT_TIMESTAMP);

  CheckIntInfo(SQL_CONVERT_DECIMAL,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
                   | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_TIMESTAMP
                   | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_SMALLINT
                   | SQL_CVT_INTEGER | SQL_CVT_BIGINT | SQL_CVT_REAL
                   | SQL_CVT_FLOAT | SQL_CVT_DOUBLE | SQL_CVT_BINARY
                   | SQL_CVT_VARBINARY | SQL_CVT_LONGVARBINARY);

  CheckIntInfo(SQL_CONVERT_DOUBLE,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
                   | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_TIMESTAMP
                   | SQL_CVT_TINYINT | SQL_CVT_BIGINT | SQL_CVT_INTEGER
                   | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE
                   | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_SMALLINT
                   | SQL_CVT_LONGVARBINARY);

  CheckIntInfo(SQL_CONVERT_FLOAT,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WVARCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_NUMERIC
                   | SQL_CVT_DECIMAL | SQL_CVT_TINYINT | SQL_CVT_SMALLINT
                   | SQL_CVT_INTEGER | SQL_CVT_BIGINT | SQL_CVT_REAL
                   | SQL_CVT_FLOAT | SQL_CVT_DOUBLE | SQL_CVT_BINARY
                   | SQL_CVT_VARBINARY | SQL_CVT_WCHAR | SQL_CVT_LONGVARBINARY
                   | SQL_CVT_TIMESTAMP | SQL_CVT_BIT);

  CheckIntInfo(SQL_CONVERT_REAL,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL
                   | SQL_CVT_BIT | SQL_CVT_FLOAT | SQL_CVT_SMALLINT
                   | SQL_CVT_REAL | SQL_CVT_INTEGER | SQL_CVT_BIGINT
                   | SQL_CVT_DOUBLE | SQL_CVT_TINYINT | SQL_CVT_WLONGVARCHAR
                   | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_LONGVARBINARY
                   | SQL_CVT_TIMESTAMP | SQL_CVT_WCHAR);

  CheckIntInfo(SQL_CONVERT_INTEGER,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
                   | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_TINYINT
                   | SQL_CVT_SMALLINT | SQL_CVT_BIT | SQL_CVT_INTEGER
                   | SQL_CVT_BIGINT | SQL_CVT_REAL | SQL_CVT_FLOAT
                   | SQL_CVT_DOUBLE | SQL_CVT_BINARY | SQL_CVT_VARBINARY
                   | SQL_CVT_LONGVARBINARY | SQL_CVT_TIMESTAMP);

  CheckIntInfo(SQL_CONVERT_NUMERIC,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
                   | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_SMALLINT
                   | SQL_CVT_INTEGER | SQL_CVT_BIGINT | SQL_CVT_REAL
                   | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_FLOAT
                   | SQL_CVT_DOUBLE | SQL_CVT_BINARY | SQL_CVT_VARBINARY
                   | SQL_CVT_LONGVARBINARY | SQL_CVT_TIMESTAMP);

  CheckIntInfo(SQL_CONVERT_SMALLINT,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
                   | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_VARBINARY
                   | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_SMALLINT
                   | SQL_CVT_INTEGER | SQL_CVT_BIGINT | SQL_CVT_REAL
                   | SQL_CVT_FLOAT | SQL_CVT_DOUBLE | SQL_CVT_BINARY
                   | SQL_CVT_LONGVARBINARY | SQL_CVT_TIMESTAMP);

  CheckIntInfo(SQL_CONVERT_TINYINT,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
                   | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_TINYINT
                   | SQL_CVT_SMALLINT | SQL_CVT_BIT | SQL_CVT_INTEGER
                   | SQL_CVT_BIGINT | SQL_CVT_REAL | SQL_CVT_FLOAT
                   | SQL_CVT_DOUBLE | SQL_CVT_BINARY | SQL_CVT_VARBINARY
                   | SQL_CVT_LONGVARBINARY | SQL_CVT_TIMESTAMP);

  CheckIntInfo(SQL_CONVERT_TIME,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_WVARCHAR | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR
                   | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_LONGVARBINARY
                   | SQL_CVT_TIME | SQL_CVT_TIMESTAMP);

  CheckIntInfo(SQL_CONVERT_TIMESTAMP,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_DATE | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR
                   | SQL_CVT_WVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_LONGVARBINARY
                   | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_BINARY
                   | SQL_CVT_VARBINARY | SQL_CVT_TIMESTAMP | SQL_CVT_BIGINT
                   | SQL_CVT_TIME);

  CheckIntInfo(SQL_CONVERT_VARBINARY,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
                   | SQL_CVT_BIT | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR
                   | SQL_CVT_WVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL
                   | SQL_CVT_TINYINT | SQL_CVT_SMALLINT | SQL_CVT_DATE
                   | SQL_CVT_BIGINT | SQL_CVT_REAL | SQL_CVT_FLOAT
                   | SQL_CVT_DOUBLE | SQL_CVT_INTEGER | SQL_CVT_BINARY
                   | SQL_CVT_VARBINARY | SQL_CVT_LONGVARBINARY | SQL_CVT_TIME
                   | SQL_CVT_TIMESTAMP | SQL_CVT_GUID);

  CheckIntInfo(SQL_CONVERT_LONGVARBINARY,
               SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_BIT | SQL_CVT_TINYINT
                   | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
                   | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL
                   | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_BIGINT
                   | SQL_CVT_REAL | SQL_CVT_DATE | SQL_CVT_DOUBLE
                   | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_LONGVARBINARY
                   | SQL_CVT_TIMESTAMP | SQL_CVT_SMALLINT | SQL_CVT_TIME
                   | SQL_CVT_GUID);

  CheckIntInfo(SQL_SQL92_ROW_VALUE_CONSTRUCTOR,
               SQL_SRVC_VALUE_EXPRESSION | SQL_SRVC_DEFAULT | SQL_SRVC_NULL
                   | SQL_SRVC_ROW_SUBQUERY);

  CheckIntInfo(SQL_SUBQUERIES, SQL_SQ_CORRELATED_SUBQUERIES | SQL_SQ_COMPARISON
                                   | SQL_SQ_EXISTS | SQL_SQ_IN
                                   | SQL_SQ_QUANTIFIED);

  CheckIntInfo(SQL_FETCH_DIRECTION, SQL_FD_FETCH_NEXT | SQL_FD_FETCH_PRIOR);

  CheckShortInfo(SQL_MAX_CONCURRENT_ACTIVITIES, 0);
  CheckShortInfo(SQL_CURSOR_COMMIT_BEHAVIOR, SQL_CB_PRESERVE);
  CheckShortInfo(SQL_CURSOR_ROLLBACK_BEHAVIOR, SQL_CB_PRESERVE);
  CheckShortInfo(SQL_TXN_CAPABLE, SQL_TC_DDL_COMMIT);
  CheckShortInfo(SQL_QUOTED_IDENTIFIER_CASE, SQL_IC_SENSITIVE);
  CheckShortInfo(SQL_ACTIVE_ENVIRONMENTS, 0);
  CheckShortInfo(SQL_CONCAT_NULL_BEHAVIOR, SQL_CB_NULL);
  CheckShortInfo(SQL_CORRELATION_NAME, SQL_CN_ANY);
  CheckShortInfo(SQL_FILE_USAGE, SQL_FILE_NOT_SUPPORTED);
  CheckShortInfo(SQL_GROUP_BY, SQL_GB_GROUP_BY_EQUALS_SELECT);
  CheckShortInfo(SQL_IDENTIFIER_CASE, SQL_IC_UPPER);
  CheckShortInfo(SQL_MAX_COLUMN_NAME_LEN, 0);
  CheckShortInfo(SQL_MAX_COLUMNS_IN_GROUP_BY, 0);
  CheckShortInfo(SQL_MAX_COLUMNS_IN_INDEX, 0);
  CheckShortInfo(SQL_MAX_COLUMNS_IN_ORDER_BY, 0);
  CheckShortInfo(SQL_MAX_COLUMNS_IN_SELECT, 0);
  CheckShortInfo(SQL_MAX_COLUMNS_IN_TABLE, 0);
  CheckShortInfo(SQL_MAX_CURSOR_NAME_LEN, 0);
  CheckShortInfo(SQL_MAX_DRIVER_CONNECTIONS, 0);
  CheckShortInfo(SQL_MAX_IDENTIFIER_LEN, 0);
  CheckShortInfo(SQL_MAX_PROCEDURE_NAME_LEN, 0);
  CheckShortInfo(SQL_MAX_SCHEMA_NAME_LEN, 0);
  CheckShortInfo(SQL_MAX_TABLE_NAME_LEN, 0);
  CheckShortInfo(SQL_MAX_TABLES_IN_SELECT, 0);
  CheckShortInfo(SQL_MAX_USER_NAME_LEN, 0);
  CheckShortInfo(SQL_NON_NULLABLE_COLUMNS, SQL_NNC_NULL);
  CheckShortInfo(SQL_NULL_COLLATION, SQL_NC_END);
}

BOOST_AUTO_TEST_SUITE_END()
