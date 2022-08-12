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

#include "documentdb/odbc/impl/binary/binary_utils.h"
#include "documentdb/odbc/config/connection_info.h"
#include "documentdb/odbc/system/odbc_constants.h"
#include "documentdb/odbc/utility.h"
#include "odbc_test_suite.h"
#include "test_type.h"
#include "test_utils.h"

using namespace documentdb;
using namespace documentdb_test;

using namespace boost::unit_test;

using documentdb::odbc::impl::binary::BinaryUtils;

/**
 * Test setup fixture.
 */
struct SqlGetInfoTestSuiteFixture : odbc::OdbcTestSuite {
  void CheckStrInfo(SQLSMALLINT type, const std::string& expectedValue) {
    SQLSMALLINT valLen = 0;

    // Get required length.
    std::string typeStr = odbc::config::ConnectionInfo::InfoTypeToString(type);
    SQLRETURN ret = SQLGetInfo(dbc, type, nullptr, 0, &valLen);
    ODBC_FAIL_ON_ERROR1(ret, SQL_HANDLE_DBC, dbc, typeStr);

    // Note: length is in bytes, not characters.
    std::vector< SQLWCHAR > val((valLen / sizeof(SQLWCHAR)) + 1);
    ret = SQLGetInfo(dbc, type, val.data(), val.size() * sizeof(SQLWCHAR), &valLen);

    ODBC_FAIL_ON_ERROR1(ret, SQL_HANDLE_DBC, dbc, typeStr);
    BOOST_CHECK_EQUAL(utility::SqlWcharToString(val.data()), expectedValue);
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

  void CheckDbmsVerInfo() {
    SQLSMALLINT valLen = 0;
    const static SQLSMALLINT type = SQL_DBMS_VER;

    // Get required length.
    std::string typeStr = odbc::config::ConnectionInfo::InfoTypeToString(type);
    SQLRETURN ret = SQLGetInfo(dbc, type, nullptr, 0, &valLen);
    ODBC_FAIL_ON_ERROR1(ret, SQL_HANDLE_DBC, dbc, typeStr);

    // Note: length is in bytes, not characters.
    std::vector< SQLWCHAR > val((valLen / sizeof(SQLWCHAR)) + 1);
    ret = SQLGetInfo(dbc, type, val.data(), val.size() * sizeof(SQLWCHAR),
                     &valLen);

    ODBC_FAIL_ON_ERROR1(ret, SQL_HANDLE_DBC, dbc, typeStr);
    std::string valStr = utility::SqlWcharToString(val.data());
    size_t startLoc = 0;
    size_t dotLoc = valStr.find('.', startLoc);
    std::string majorStr;
    std::string minorStr;
    std::string buildStr;
    if (dotLoc > 0) {
      majorStr = valStr.substr(startLoc, dotLoc - startLoc);
      startLoc = dotLoc + 1;
      dotLoc = valStr.find('.', startLoc);
      if (dotLoc > 0) {
        minorStr = valStr.substr(startLoc, dotLoc - startLoc);
        startLoc = dotLoc + 1;
        dotLoc = valStr.find('.', startLoc);
        if (dotLoc > 0) {
          buildStr = valStr.substr(startLoc, dotLoc - startLoc);
        }
      }
    }
    BOOST_CHECK(
        !majorStr.empty() && majorStr.length() == 2
        && (majorStr.find_first_not_of("0123456789") == std::string::npos));
    BOOST_CHECK(
        !minorStr.empty() && minorStr.length() == 2
        && (minorStr.find_first_not_of("0123456789") == std::string::npos));
    BOOST_CHECK(
        !buildStr.empty() && buildStr.length() == 4
        && (buildStr.find_first_not_of("0123456789") == std::string::npos));
  }

  /**
   * Constructor.
   */
  SqlGetInfoTestSuiteFixture() {
  }

  /**
   * Destructor.
   */
  ~SqlGetInfoTestSuiteFixture() {
  }

  /**
   * Connect to the local server with the database name
   *
   * @param databaseName Database Name
   */
  void connectToLocalServer(std::string databaseName) {
    std::string dsnConnectionString;
    CreateDsnConnectionStringForLocalServer(dsnConnectionString, databaseName);

    Connect(dsnConnectionString);
  }
};

BOOST_FIXTURE_TEST_SUITE(SqlGetInfoTestSuite, SqlGetInfoTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestValues) {
  connectToLocalServer("odbc-test");

  CheckStrInfo(SQL_DRIVER_NAME, "Amazon DocumentDB");
  CheckStrInfo(SQL_DBMS_NAME, "Amazon DocumentDB");
  CheckStrInfo(SQL_DRIVER_ODBC_VER, "03.00");
  CheckStrInfo(SQL_DRIVER_VER, "00.01.0000");
  CheckDbmsVerInfo();
  CheckStrInfo(SQL_COLUMN_ALIAS, "Y");
  CheckStrInfo(SQL_IDENTIFIER_QUOTE_CHAR, "\"");
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
#if defined(__linux) || defined(__linux__) || defined(linux)
  // As we are connecting using SQLDriverConnect, it seems the driver is
  // removing the DSN setting
  CheckStrInfo(SQL_DATA_SOURCE_NAME, "");
#else
  CheckStrInfo(SQL_DATA_SOURCE_NAME, "DocumentDB DSN");
#endif
  CheckStrInfo(SQL_DATA_SOURCE_READ_ONLY, "Y");
  CheckStrInfo(SQL_DATABASE_NAME, "odbc-test");
  CheckStrInfo(SQL_DESCRIBE_PARAMETER, "N");
  CheckStrInfo(SQL_EXPRESSIONS_IN_ORDERBY, "Y");
  CheckStrInfo(SQL_INTEGRITY, "N");
  CheckStrInfo(
      SQL_KEYWORDS,
      "ABS,ALLOW,ARRAY,ARRAY_MAX_CARDINALITY,ASYMMETRIC,ATOMIC,BEGIN_FRAME,"
      "BEGIN_PARTITION,BIGINT,BINARY,BLOB,BOOLEAN,CALL,CALLED,CARDINALITY,CEIL,"
      "CEILING,CHAR_LENGTH,CLASSIFIER,CLOB,COLLECT,CONDITION,CORR,COVAR_POP,"
      "COVAR_SAMP,CUBE,CUME_DIST,CURRENT_CATALOG,CURRENT_DEFAULT_TRANSFORM_"
      "GROUP,CURRENT_PATH,CURRENT_ROLE,CURRENT_ROW,CURRENT_SCHEMA,CURRENT_"
      "TRANSFORM_GROUP_FOR_TYPE,CYCLE,DENSE_RANK,DEREF,DETERMINISTIC,DISALLOW,"
      "DYNAMIC,EACH,ELEMENT,EMPTY,END_FRAME,END_PARTITION,EQUALS,EVERY,EXP,"
      "EXPLAIN,EXTEND,FILTER,FIRST_VALUE,FLOOR,FRAME_ROW,FREE,FUNCTION,FUSION,"
      "GROUPING,GROUPS,HOLD,IMPORT,INITIAL,INOUT,INTERSECTION,JSON_ARRAY,JSON_"
      "ARRAYAGG,JSON_EXISTS,JSON_OBJECT,JSON_OBJECTAGG,JSON_QUERY,JSON_VALUE,"
      "LAG,LARGE,LAST_VALUE,LATERAL,LEAD,LIKE_REGEX,LIMIT,LN,LOCALTIME,"
      "LOCALTIMESTAMP,MATCHES,MATCH_NUMBER,MATCH_RECOGNIZE,MEASURES,MEMBER,"
      "MERGE,METHOD,MINUS,MOD,MODIFIES,MULTISET,NCLOB,NEW,NORMALIZE,NTH_VALUE,"
      "NTILE,OCCURRENCES_REGEX,OFFSET,OLD,OMIT,ONE,OUT,OVER,OVERLAY,PARAMETER,"
      "PARTITION,PATTERN,PER,PERCENT,PERCENTILE_CONT,PERCENTILE_DISC,PERCENT_"
      "RANK,PERIOD,PERMUTE,PORTION,POSITION_REGEX,POWER,PRECEDES,PREV,RANGE,"
      "RANK,READS,RECURSIVE,REF,REFERENCING,REGR_AVGX,REGR_AVGY,REGR_COUNT,"
      "REGR_INTERCEPT,REGR_R2,REGR_SLOPE,REGR_SXX,REGR_SXY,REGR_SYY,RELEASE,"
      "RESET,RESULT,RETURN,RETURNS,ROLLUP,ROW,ROW_NUMBER,RUNNING,SAVEPOINT,"
      "SCOPE,SEARCH,SEEK,SENSITIVE,SHOW,SIMILAR,SKIP,SPECIFIC,SPECIFICTYPE,"
      "SQLEXCEPTION,SQRT,START,STATIC,STDDEV_POP,STDDEV_SAMP,STREAM,"
      "SUBMULTISET,SUBSET,SUBSTRING_REGEX,SUCCEEDS,SYMMETRIC,SYSTEM,SYSTEM_"
      "TIME,TABLESAMPLE,TINYINT,TRANSLATE_REGEX,TREAT,TRIGGER,TRIM_ARRAY,"
      "TRUNCATE,UESCAPE,UNNEST,UPSERT,VALUE_OF,VARBINARY,VAR_POP,VAR_SAMP,"
      "VERSIONING,WIDTH_BUCKET,WINDOW,WITHIN,WITHOUT");
  CheckStrInfo(SQL_LIKE_ESCAPE_CLAUSE, "N");
  CheckStrInfo(SQL_MAX_ROW_SIZE_INCLUDES_LONG, "Y");
  CheckStrInfo(SQL_MULT_RESULT_SETS, "N");
  CheckStrInfo(SQL_MULTIPLE_ACTIVE_TXN, "Y");
  CheckStrInfo(SQL_ORDER_BY_COLUMNS_IN_SELECT, "N");
  CheckStrInfo(SQL_PROCEDURE_TERM, "stored procedure");
  CheckStrInfo(SQL_PROCEDURES, "N");
  CheckStrInfo(SQL_ROW_UPDATES, "N");
  CheckStrInfo(SQL_SEARCH_PATTERN_ESCAPE, "\\");
  CheckStrInfo(SQL_SERVER_NAME, "Amazon DocumentDB");
  std::string expectedUserName = common::GetEnv("DOC_DB_USER_NAME", "documentdb");
  CheckStrInfo(SQL_USER_NAME, expectedUserName);

  CheckIntInfo(SQL_ASYNC_MODE, SQL_AM_NONE);
  CheckIntInfo(SQL_BATCH_ROW_COUNT, SQL_BRC_ROLLED_UP | SQL_BRC_EXPLICIT);
  CheckIntInfo(SQL_BATCH_SUPPORT, 0);
  CheckIntInfo(SQL_BOOKMARK_PERSISTENCE, 0);
  CheckIntInfo(SQL_CATALOG_LOCATION, 0);
  CheckIntInfo(SQL_QUALIFIER_LOCATION, 0);
  CheckIntInfo(SQL_GETDATA_EXTENSIONS,
               SQL_GD_ANY_COLUMN | SQL_GD_ANY_ORDER | SQL_GD_BOUND);
  CheckIntInfo(SQL_ODBC_INTERFACE_CONFORMANCE, SQL_OIC_CORE);
  CheckIntInfo(SQL_SQL_CONFORMANCE, SQL_SC_SQL92_ENTRY);
  CheckIntInfo(SQL_CATALOG_USAGE, 0);
  CheckIntInfo(SQL_QUALIFIER_USAGE, 0);
  CheckIntInfo(SQL_TIMEDATE_ADD_INTERVALS,
               SQL_FN_TSI_FRAC_SECOND | SQL_FN_TSI_SECOND | SQL_FN_TSI_MINUTE
                   | SQL_FN_TSI_HOUR | SQL_FN_TSI_DAY | SQL_FN_TSI_WEEK
                   | SQL_FN_TSI_MONTH | SQL_FN_TSI_QUARTER | SQL_FN_TSI_YEAR);
  CheckIntInfo(SQL_TIMEDATE_DIFF_INTERVALS,
               SQL_FN_TSI_FRAC_SECOND | SQL_FN_TSI_SECOND | SQL_FN_TSI_MINUTE
                   | SQL_FN_TSI_HOUR | SQL_FN_TSI_DAY | SQL_FN_TSI_WEEK
                   | SQL_FN_TSI_MONTH | SQL_FN_TSI_QUARTER | SQL_FN_TSI_YEAR);
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
  CheckIntInfo(SQL_STATIC_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT);
  CheckIntInfo(SQL_STATIC_CURSOR_ATTRIBUTES2, 0);
  CheckIntInfo(SQL_PARAM_ARRAY_ROW_COUNTS, SQL_PARC_BATCH);
  CheckIntInfo(SQL_PARAM_ARRAY_SELECTS, SQL_PAS_NO_SELECT);
  CheckIntInfo(SQL_SCROLL_OPTIONS, SQL_SO_FORWARD_ONLY | SQL_SO_STATIC);
  CheckIntInfo(SQL_ALTER_DOMAIN, 0);
  CheckIntInfo(SQL_ALTER_TABLE, 0);
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
  CheckIntInfo(SQL_DDL_INDEX, 0);
  CheckIntInfo(SQL_DEFAULT_TXN_ISOLATION, SQL_TXN_REPEATABLE_READ);
  CheckIntInfo(SQL_DROP_ASSERTION, 0);
  CheckIntInfo(SQL_DROP_CHARACTER_SET, 0);
  CheckIntInfo(SQL_DROP_COLLATION, 0);
  CheckIntInfo(SQL_DROP_DOMAIN, 0);
  CheckIntInfo(SQL_DROP_SCHEMA, 0);
  CheckIntInfo(SQL_DROP_TABLE, 0);
  CheckIntInfo(SQL_DROP_TRANSLATION, 0);
  CheckIntInfo(SQL_DROP_VIEW, 0);
  CheckIntInfo(SQL_DYNAMIC_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT);
  CheckIntInfo(SQL_DYNAMIC_CURSOR_ATTRIBUTES2, SQL_CA2_READ_ONLY_CONCURRENCY);
  CheckIntInfo(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT);
  CheckIntInfo(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2,
               SQL_CA2_READ_ONLY_CONCURRENCY);
  CheckIntInfo(SQL_INDEX_KEYWORDS, 0);
  CheckIntInfo(SQL_INFO_SCHEMA_VIEWS, 0);
  CheckIntInfo(SQL_INSERT_STATEMENT, 0);
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
  CheckIntInfo(SQL_UNION, 0);

  CheckIntInfo(SQL_SCHEMA_USAGE, SQL_SU_DML_STATEMENTS | SQL_SU_TABLE_DEFINITION
                                     | SQL_SU_PRIVILEGE_DEFINITION
                                     | SQL_SU_INDEX_DEFINITION);

  CheckIntInfo(SQL_AGGREGATE_FUNCTIONS, SQL_AF_AVG | SQL_AF_COUNT | SQL_AF_MAX
                                            | SQL_AF_MIN | SQL_AF_SUM
                                            | SQL_AF_DISTINCT | SQL_AF_ALL);

  CheckIntInfo(SQL_NUMERIC_FUNCTIONS, SQL_FN_NUM_MOD);

  CheckIntInfo(SQL_STRING_FUNCTIONS,
               SQL_FN_STR_CHAR_LENGTH | SQL_FN_STR_CONCAT | SQL_FN_STR_LEFT
                   | SQL_FN_STR_LOCATE | SQL_FN_STR_CHARACTER_LENGTH
                   | SQL_FN_STR_POSITION | SQL_FN_STR_RIGHT
                   | SQL_FN_STR_SUBSTRING | SQL_FN_STR_LCASE
                   | SQL_FN_STR_UCASE);

  CheckIntInfo(SQL_TIMEDATE_FUNCTIONS,
               SQL_FN_TD_CURRENT_TIME | SQL_FN_TD_CURRENT_DATE
                   | SQL_FN_TD_CURRENT_TIMESTAMP | SQL_FN_TD_EXTRACT
                   | SQL_FN_TD_YEAR | SQL_FN_TD_QUARTER | SQL_FN_TD_MONTH
                   | SQL_FN_TD_WEEK | SQL_FN_TD_DAYOFYEAR | SQL_FN_TD_DAYOFMONTH
                   | SQL_FN_TD_DAYOFWEEK | SQL_FN_TD_HOUR | SQL_FN_TD_MINUTE
                   | SQL_FN_TD_SECOND | SQL_FN_TD_DAYNAME
                   | SQL_FN_TD_MONTHNAME);

  CheckIntInfo(SQL_SQL92_NUMERIC_VALUE_FUNCTIONS, SQL_SNVF_CHARACTER_LENGTH
                                                      | SQL_SNVF_EXTRACT
                                                      | SQL_SNVF_POSITION);

  CheckIntInfo(SQL_SQL92_STRING_FUNCTIONS,
               SQL_SSF_LOWER | SQL_SSF_UPPER | SQL_SSF_SUBSTRING);

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

  CheckIntInfo(SQL_SUBQUERIES, 0);

  CheckIntInfo(SQL_FETCH_DIRECTION, SQL_FD_FETCH_NEXT);

  CheckShortInfo(SQL_MAX_CONCURRENT_ACTIVITIES, 0);
  CheckShortInfo(SQL_QUOTED_IDENTIFIER_CASE, SQL_IC_SENSITIVE);
  CheckShortInfo(SQL_ACTIVE_ENVIRONMENTS, 0);
  CheckShortInfo(SQL_CONCAT_NULL_BEHAVIOR, SQL_CB_NULL);
  CheckShortInfo(SQL_CORRELATION_NAME, SQL_CN_ANY);
  CheckShortInfo(SQL_FILE_USAGE, SQL_FILE_NOT_SUPPORTED);
  CheckShortInfo(SQL_GROUP_BY, SQL_GB_GROUP_BY_EQUALS_SELECT);
  CheckShortInfo(SQL_IDENTIFIER_CASE, SQL_IC_SENSITIVE);
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
  CheckShortInfo(SQL_NON_NULLABLE_COLUMNS, SQL_NNC_NON_NULL);
  CheckShortInfo(SQL_NULL_COLLATION, SQL_NC_LOW);
  CheckShortInfo(SQL_TXN_CAPABLE, SQL_TC_NONE);
}

BOOST_AUTO_TEST_SUITE_END()
