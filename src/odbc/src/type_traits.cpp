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

#include "ignite/odbc/type_traits.h"

#include <ignite/impl/binary/binary_common.h>

#include "ignite/odbc/system/odbc_constants.h"

using namespace ignite::impl::binary;

namespace ignite {
namespace odbc {
namespace type_traits {
const std::string SqlTypeName::SMALLINT("SMALLINT");

const std::string SqlTypeName::INTEGER("INTEGER");

const std::string SqlTypeName::DECIMAL("DECIMAL");

const std::string SqlTypeName::FLOAT("FLOAT");

const std::string SqlTypeName::REAL("REAL");

const std::string SqlTypeName::DOUBLE("DOUBLE");

const std::string SqlTypeName::NUMERIC("NUMERIC");

const std::string SqlTypeName::BIT("BIT");

const std::string SqlTypeName::TINYINT("TINYINT");

const std::string SqlTypeName::BIGINT("BIGINT");

const std::string SqlTypeName::VARCHAR("VARCHAR");

const std::string SqlTypeName::LONGVARCHAR("LONGVARCHAR");

const std::string SqlTypeName::BINARY("VARBINARY");

const std::string SqlTypeName::LONGVARBINARY("LONGVARBINARY");

const std::string SqlTypeName::DATE("DATE");

const std::string SqlTypeName::TIMESTAMP("TIMESTAMP");

const std::string SqlTypeName::TIME("TIME");

const std::string SqlTypeName::GUID("GUID");

const std::string SqlTypeName::SQL_NULL("NULL");

#ifdef _DEBUG

#define DBG_STR_CASE(x) \
  case x:               \
    return #x

const char* StatementAttrIdToString(long id) {
  switch (id) {
    DBG_STR_CASE(SQL_ATTR_APP_PARAM_DESC);
    DBG_STR_CASE(SQL_ATTR_APP_ROW_DESC);
    DBG_STR_CASE(SQL_ATTR_ASYNC_ENABLE);
    DBG_STR_CASE(SQL_ATTR_CONCURRENCY);
    DBG_STR_CASE(SQL_ATTR_CURSOR_SCROLLABLE);
    DBG_STR_CASE(SQL_ATTR_CURSOR_SENSITIVITY);
    DBG_STR_CASE(SQL_ATTR_CURSOR_TYPE);
    DBG_STR_CASE(SQL_ATTR_ENABLE_AUTO_IPD);
    DBG_STR_CASE(SQL_ATTR_FETCH_BOOKMARK_PTR);
    DBG_STR_CASE(SQL_ATTR_IMP_PARAM_DESC);
    DBG_STR_CASE(SQL_ATTR_IMP_ROW_DESC);
    DBG_STR_CASE(SQL_ATTR_KEYSET_SIZE);
    DBG_STR_CASE(SQL_ATTR_MAX_LENGTH);
    DBG_STR_CASE(SQL_ATTR_MAX_ROWS);
    DBG_STR_CASE(SQL_ATTR_METADATA_ID);
    DBG_STR_CASE(SQL_ATTR_NOSCAN);
    DBG_STR_CASE(SQL_ATTR_PARAM_BIND_OFFSET_PTR);
    DBG_STR_CASE(SQL_ATTR_PARAM_BIND_TYPE);
    DBG_STR_CASE(SQL_ATTR_PARAM_OPERATION_PTR);
    DBG_STR_CASE(SQL_ATTR_PARAM_STATUS_PTR);
    DBG_STR_CASE(SQL_ATTR_PARAMS_PROCESSED_PTR);
    DBG_STR_CASE(SQL_ATTR_PARAMSET_SIZE);
    DBG_STR_CASE(SQL_ATTR_QUERY_TIMEOUT);
    DBG_STR_CASE(SQL_ATTR_RETRIEVE_DATA);
    DBG_STR_CASE(SQL_ATTR_ROW_ARRAY_SIZE);
    DBG_STR_CASE(SQL_ATTR_ROW_BIND_OFFSET_PTR);
    DBG_STR_CASE(SQL_ATTR_ROW_BIND_TYPE);
    DBG_STR_CASE(SQL_ATTR_ROW_NUMBER);
    DBG_STR_CASE(SQL_ATTR_ROW_OPERATION_PTR);
    DBG_STR_CASE(SQL_ATTR_ROW_STATUS_PTR);
    DBG_STR_CASE(SQL_ATTR_ROWS_FETCHED_PTR);
    DBG_STR_CASE(SQL_ATTR_SIMULATE_CURSOR);
    DBG_STR_CASE(SQL_ATTR_USE_BOOKMARKS);
    default:
      break;
  }
  return "<< UNKNOWN ID >>";
}

#undef DBG_STR_CASE
#endif  // _DEBUG
/**
 * Warning: if any JDBC Type is added or becomes deprecated on the JDBC side,
 * the change should be reflected under this function as well.
 */
const std::string& BinaryTypeToSqlTypeName(int16_t binaryType) {
  switch (binaryType) {
    case JDBC_TYPE_BIT:
    case JDBC_TYPE_BOOLEAN:
      return SqlTypeName::BIT;

    case JDBC_TYPE_SMALLINT:
      return SqlTypeName::SMALLINT;

    case JDBC_TYPE_TINYINT:
      return SqlTypeName::TINYINT;

    case JDBC_TYPE_INTEGER:
      return SqlTypeName::INTEGER;

    case JDBC_TYPE_BIGINT:
      return SqlTypeName::BIGINT;

    case JDBC_TYPE_FLOAT:
      return SqlTypeName::FLOAT;

    case JDBC_TYPE_REAL:
      return SqlTypeName::REAL;

    case JDBC_TYPE_DOUBLE:
      return SqlTypeName::DOUBLE;

    case JDBC_TYPE_NUMERIC:
      return SqlTypeName::NUMERIC;

    case JDBC_TYPE_DECIMAL:
      return SqlTypeName::DECIMAL;

    case JDBC_TYPE_VARCHAR:
    case JDBC_TYPE_NVARCHAR:
      return SqlTypeName::VARCHAR;

    case JDBC_TYPE_LONGVARCHAR:
    case JDBC_TYPE_LONGNVARCHAR:
      return SqlTypeName::LONGVARCHAR;

    case JDBC_TYPE_DATE:
      return SqlTypeName::DATE;

    case JDBC_TYPE_TIME:
      return SqlTypeName::TIME;

    case JDBC_TYPE_TIMESTAMP:
      return SqlTypeName::TIMESTAMP;

    case JDBC_TYPE_LONGVARBINARY:
      return SqlTypeName::LONGVARBINARY;

    case JDBC_TYPE_NULL:
      return SqlTypeName::SQL_NULL;

    case JDBC_TYPE_BINARY:
    case JDBC_TYPE_VARBINARY:
    case JDBC_TYPE_BLOB:
    case JDBC_TYPE_CLOB:
    case JDBC_TYPE_ARRAY:
    case JDBC_TYPE_STRUCT:
    case JDBC_TYPE_JAVA_OBJECT:
    case JDBC_TYPE_ROWID:
    case JDBC_TYPE_NCLOB:
    case JDBC_TYPE_SQLXML:
    case JDBC_TYPE_REF_CURSOR:
    default:
      return SqlTypeName::BINARY;
  }
}

bool IsApplicationTypeSupported(int16_t type) {
  return ToDriverType(type) != OdbcNativeType::AI_UNSUPPORTED;
}

bool IsSqlTypeSupported(int16_t type) {
  switch (type) {
    case SQL_BIT:
    case SQL_TINYINT:
    case SQL_SMALLINT:
    case SQL_BIGINT:
    case SQL_INTEGER:
    case SQL_FLOAT:
    case SQL_REAL:
    case SQL_DOUBLE:
    case SQL_NUMERIC:
    case SQL_DECIMAL:
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
    case SQL_TYPE_DATE:
    case SQL_TYPE_TIMESTAMP:
    case SQL_TYPE_TIME:
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
    case SQL_TYPE_NULL:
      return true;

    case SQL_WCHAR:
    case SQL_WVARCHAR:
    case SQL_WLONGVARCHAR:
    case SQL_GUID:
    case SQL_INTERVAL_MONTH:
    case SQL_INTERVAL_YEAR:
    case SQL_INTERVAL_YEAR_TO_MONTH:
    case SQL_INTERVAL_DAY:
    case SQL_INTERVAL_HOUR:
    case SQL_INTERVAL_MINUTE:
    case SQL_INTERVAL_SECOND:
    case SQL_INTERVAL_DAY_TO_HOUR:
    case SQL_INTERVAL_DAY_TO_MINUTE:
    case SQL_INTERVAL_DAY_TO_SECOND:
    case SQL_INTERVAL_HOUR_TO_MINUTE:
    case SQL_INTERVAL_HOUR_TO_SECOND:
    case SQL_INTERVAL_MINUTE_TO_SECOND:
    default:
      return false;
  }
}

/**
 * Warning: if any JDBC Type is added or becomes deprecated on the
 * JDBC side, the change should be reflected under this function as
 * well.
 */
int16_t SqlTypeToBinary(int16_t sqlType) {
  switch (sqlType) {
    case SQL_BIT:
      return JDBC_TYPE_BOOLEAN;

    case SQL_TINYINT:
      return JDBC_TYPE_TINYINT;

    case SQL_SMALLINT:
      return JDBC_TYPE_SMALLINT;

    case SQL_INTEGER:
      return JDBC_TYPE_INTEGER;

    case SQL_BIGINT:
      return JDBC_TYPE_BIGINT;

    case SQL_FLOAT:
      return JDBC_TYPE_FLOAT;

    case SQL_REAL:
      return JDBC_TYPE_REAL;

    case SQL_DOUBLE:
      return JDBC_TYPE_DOUBLE;

    case SQL_NUMERIC:
      return JDBC_TYPE_NUMERIC;

    case SQL_DECIMAL:
      return JDBC_TYPE_DECIMAL;

    case SQL_CHAR:
      return JDBC_TYPE_CHAR;

    case SQL_VARCHAR:
      return JDBC_TYPE_VARCHAR;

    case SQL_LONGVARCHAR:
      return JDBC_TYPE_LONGVARCHAR;

    case SQL_TYPE_DATE:
      return JDBC_TYPE_DATE;

    case SQL_TYPE_TIME:
      return JDBC_TYPE_TIME;

    case SQL_TYPE_TIMESTAMP:
      return JDBC_TYPE_TIMESTAMP;

    case SQL_BINARY:
      return JDBC_TYPE_BINARY;

    case SQL_VARBINARY:
      return JDBC_TYPE_VARBINARY;

    case SQL_LONGVARBINARY:
      return JDBC_TYPE_LONGVARBINARY;

    case SQL_TYPE_NULL:
      return JDBC_TYPE_NULL;

    default:
      return JDBC_TYPE_BINARY;
  }
}

OdbcNativeType::Type ToDriverType(int16_t type) {
  switch (type) {
    case SQL_C_CHAR:
      return OdbcNativeType::AI_CHAR;

    case SQL_C_WCHAR:
      return OdbcNativeType::AI_WCHAR;

    case SQL_C_SSHORT:
    case SQL_C_SHORT:
      return OdbcNativeType::AI_SIGNED_SHORT;

    case SQL_C_USHORT:
      return OdbcNativeType::AI_UNSIGNED_SHORT;

    case SQL_C_SLONG:
    case SQL_C_LONG:
      return OdbcNativeType::AI_SIGNED_LONG;

    case SQL_C_ULONG:
      return OdbcNativeType::AI_UNSIGNED_LONG;

    case SQL_C_FLOAT:
      return OdbcNativeType::AI_FLOAT;

    case SQL_C_DOUBLE:
      return OdbcNativeType::AI_DOUBLE;

    case SQL_C_BIT:
      return OdbcNativeType::AI_BIT;

    case SQL_C_STINYINT:
    case SQL_C_TINYINT:
      return OdbcNativeType::AI_SIGNED_TINYINT;

    case SQL_C_UTINYINT:
      return OdbcNativeType::AI_UNSIGNED_TINYINT;

    case SQL_C_SBIGINT:
      return OdbcNativeType::AI_SIGNED_BIGINT;

    case SQL_C_UBIGINT:
      return OdbcNativeType::AI_UNSIGNED_BIGINT;

    case SQL_C_BINARY:
      return OdbcNativeType::AI_BINARY;

    case SQL_C_DATE:
    case SQL_C_TYPE_DATE:
      return OdbcNativeType::AI_TDATE;

    case SQL_C_TIME:
    case SQL_C_TYPE_TIME:
      return OdbcNativeType::AI_TTIME;

    case SQL_C_TIMESTAMP:
    case SQL_C_TYPE_TIMESTAMP:
      return OdbcNativeType::AI_TTIMESTAMP;

    case SQL_C_NUMERIC:
      return OdbcNativeType::AI_NUMERIC;

    case SQL_C_GUID:
      return OdbcNativeType::AI_GUID;

    case SQL_C_DEFAULT:
      return OdbcNativeType::AI_DEFAULT;

    default:
      return OdbcNativeType::AI_UNSUPPORTED;
  }
}

/**
 * Warning: if any JDBC Type is added or becomes deprecated on the
 * JDBC side, the change should be reflected under this function as
 * well.
 */
int16_t BinaryToSqlType(int16_t binaryType) {
  switch (binaryType) {
    case JDBC_TYPE_BIT:
    case JDBC_TYPE_BOOLEAN:
      return SQL_BIT;

    case JDBC_TYPE_TINYINT:
      return SQL_TINYINT;

    case JDBC_TYPE_SMALLINT:
      return SQL_SMALLINT;

    case JDBC_TYPE_INTEGER:
      return SQL_INTEGER;

    case JDBC_TYPE_BIGINT:
      return SQL_BIGINT;

    case JDBC_TYPE_FLOAT:
      return SQL_FLOAT;

    case JDBC_TYPE_REAL:
      return SQL_REAL;

    case JDBC_TYPE_DOUBLE:
      return SQL_DOUBLE;

    case JDBC_TYPE_NUMERIC:
      return SQL_NUMERIC;

    case JDBC_TYPE_DECIMAL:
      return SQL_DECIMAL;

    case JDBC_TYPE_CHAR:
    case JDBC_TYPE_NCHAR:
      return SQL_CHAR;

    case JDBC_TYPE_VARCHAR:
    case JDBC_TYPE_NVARCHAR:
      return SQL_VARCHAR;

    case JDBC_TYPE_LONGVARCHAR:
    case JDBC_TYPE_LONGNVARCHAR:
      return SQL_LONGVARCHAR;

    case JDBC_TYPE_DATE:
      return SQL_TYPE_DATE;

    case JDBC_TYPE_TIME:
      return SQL_TYPE_TIME;

    case JDBC_TYPE_TIMESTAMP:
      return SQL_TYPE_TIMESTAMP;

    case JDBC_TYPE_LONGVARBINARY:
      return SQL_LONGVARBINARY;

    case JDBC_TYPE_NULL:
      return SQL_TYPE_NULL;

    case JDBC_TYPE_BINARY:
    case JDBC_TYPE_VARBINARY:
    case JDBC_TYPE_BLOB:
    case JDBC_TYPE_CLOB:
    case JDBC_TYPE_ARRAY:
    case JDBC_TYPE_STRUCT:
    case JDBC_TYPE_JAVA_OBJECT:
    case JDBC_TYPE_ROWID:
    case JDBC_TYPE_NCLOB:
    case JDBC_TYPE_SQLXML:
    case JDBC_TYPE_REF_CURSOR:
    default:
      return SQL_BINARY;
  }
}

int16_t BinaryTypeNullability(int16_t) {
  return SQL_NULLABLE_UNKNOWN;
}

const std::string NullabiltyToIsNullable(int16_t nullability) {
  switch (nullability) {
    case SQL_NO_NULLS:
      return "NO";

    case SQL_NULLABLE:
      return "YES";

    default:
      return "";
  }
}

int32_t SqlTypeDisplaySize(int16_t type) {
  switch (type) {
    case SQL_VARCHAR:
    case SQL_CHAR:
    case SQL_WCHAR:
    case SQL_LONGVARCHAR:
    case SQL_LONGVARBINARY:
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
      return SQL_NO_TOTAL;

    case SQL_BIT:
    case SQL_TYPE_NULL:
      return 1;

    case SQL_TINYINT:
      return 4;

    case SQL_SMALLINT:
      return 6;

    case SQL_INTEGER:
      return 11;

    case SQL_BIGINT:
      return 20;

    case SQL_REAL:
      return 14;

    case SQL_FLOAT:
    case SQL_DOUBLE:
      return 24;

    case SQL_TYPE_DATE:
      return 10;

    case SQL_TYPE_TIME:
      return 8;

    case SQL_TYPE_TIMESTAMP:
      return 19;

    case SQL_GUID:
      return 36;

    default:
      return SQL_NO_TOTAL;
  }
}

int32_t BinaryTypeDisplaySize(int16_t type) {
  int16_t sqlType = BinaryToSqlType(type);

  return SqlTypeDisplaySize(sqlType);
}

int32_t SqlTypeColumnSize(int16_t type) {
  switch (type) {
    case SQL_VARCHAR:
    case SQL_CHAR:
    case SQL_WCHAR:
    case SQL_LONGVARCHAR:
    case SQL_LONGVARBINARY:
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
      return SQL_NO_TOTAL;

    case SQL_BIT:
    case SQL_TYPE_NULL:
      return 1;

    case SQL_TINYINT:
      return 3;

    case SQL_SMALLINT:
      return 5;

    case SQL_INTEGER:
      return 10;

    case SQL_BIGINT:
      return 19;

    case SQL_REAL:
      return 7;

    case SQL_FLOAT:
    case SQL_DOUBLE:
      return 15;

    case SQL_TYPE_DATE:
      return 10;

    case SQL_TYPE_TIME:
      return 8;

    case SQL_TYPE_TIMESTAMP:
      return 19;

    case SQL_GUID:
      return 36;

    default:
      return SQL_NO_TOTAL;
  }
}

int32_t BinaryTypeColumnSize(int16_t type) {
  int16_t sqlType = BinaryToSqlType(type);

  return SqlTypeColumnSize(sqlType);
}

int32_t SqlTypeTransferLength(int16_t type) {
  switch (type) {
    case SQL_VARCHAR:
    case SQL_CHAR:
    case SQL_WCHAR:
    case SQL_LONGVARCHAR:
    case SQL_LONGVARBINARY:
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
      return SQL_NO_TOTAL;

    case SQL_BIT:
    case SQL_TINYINT:
    case SQL_TYPE_NULL:
      return 1;

    case SQL_SMALLINT:
      return 2;

    case SQL_INTEGER:
      return 4;

    case SQL_BIGINT:
      return 8;

    case SQL_REAL:
      return 4;

    case SQL_FLOAT:
    case SQL_DOUBLE:
      return 8;

    case SQL_TYPE_DATE:
    case SQL_TYPE_TIME:
      return 6;

    case SQL_TYPE_TIMESTAMP:
      return 16;

    case SQL_GUID:
      return 16;

    default:
      return SQL_NO_TOTAL;
  }
}

int32_t BinaryTypeTransferLength(int16_t type) {
  int16_t sqlType = BinaryToSqlType(type);

  return SqlTypeTransferLength(sqlType);
}

int32_t SqlTypeNumPrecRadix(int16_t type) {
  switch (type) {
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
      return 2;

    case SQL_BIT:
    case SQL_TINYINT:
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_BIGINT:
      return 10;

    default:
      return -1;
  }
}

int32_t BinaryTypeNumPrecRadix(int16_t type) {
  int16_t sqlType = BinaryToSqlType(type);

  return SqlTypeNumPrecRadix(sqlType);
}

int32_t SqlTypeDecimalDigits(int16_t) {
  // Not implemented for the NUMERIC and DECIMAL data types.
  return -1;
}

int32_t BinaryTypeDecimalDigits(int16_t type) {
  int16_t sqlType = BinaryToSqlType(type);

  return SqlTypeDecimalDigits(sqlType);
}

int32_t SqlTypeCharOctetLength(int16_t type) {
  switch (type) {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
    case SQL_BINARY:
    case SQL_LONGVARBINARY:
      return SQL_NO_TOTAL;

    default:
      return 0;
  }
}

int32_t BinaryTypeCharOctetLength(int16_t type) {
  int16_t sqlType = BinaryToSqlType(type);

  return SqlTypeCharOctetLength(sqlType);
}

bool SqlTypeUnsigned(int16_t type) {
  switch (type) {
    case SQL_BIT:
    case SQL_TINYINT:
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_BIGINT:
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
      return false;

    default:
      return true;
  }
}

bool BinaryTypeUnsigned(int16_t type) {
  int16_t sqlType = BinaryToSqlType(type);

  return SqlTypeUnsigned(sqlType);
}
}  // namespace type_traits
}  // namespace odbc
}  // namespace ignite
