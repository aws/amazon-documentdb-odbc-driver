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

#include "documentdb/odbc/query/type_info_query.h"

#include <cassert>

#include "documentdb/odbc/connection.h"
#include "documentdb/odbc/jni/database_metadata.h"
#include "documentdb/odbc/impl/binary/binary_common.h"
#include "documentdb/odbc/system/odbc_constants.h"
#include "documentdb/odbc/type_traits.h"

using namespace documentdb::odbc::jni;

namespace {
struct ResultColumn {
  enum Type {
    /** Data source-dependent data-type name. */
    TYPE_NAME = 1,

    /** SQL data type. */
    DATA_TYPE,

    /** The maximum column size that the server supports for this data type. */
    COLUMN_SIZE,

    /** Character or characters used to prefix a literal. */
    LITERAL_PREFIX,

    /** Character or characters used to terminate a literal. */
    LITERAL_SUFFIX,

    /**
     * A list of keywords, separated by commas, corresponding to each
     * parameter that the application may specify in parentheses when using
     * the name that is returned in the TYPE_NAME field.
     */
    CREATE_PARAMS,

    /** Whether the data type accepts a NULL value. */
    NULLABLE,

    /**
     * Whether a character data type is case-sensitive in collations and
     * comparisons.
     */
    CASE_SENSITIVE,

    /** How the data type is used in a WHERE clause. */
    SEARCHABLE,

    /** Whether the data type is unsigned. */
    UNSIGNED_ATTRIBUTE,

    /** Whether the data type has predefined fixed precision and scale. */
    FIXED_PREC_SCALE,

    /** Whether the data type is auto-incrementing. */
    AUTO_UNIQUE_VALUE,

    /**
     * Localized version of the data source–dependent name of the data
     * type.
     */
    LOCAL_TYPE_NAME,

    /** The minimum scale of the data type on the data source. */
    MINIMUM_SCALE,

    /** The maximum scale of the data type on the data source. */
    MAXIMUM_SCALE,

    /**
     * The value of the SQL data type as it appears in the SQL_DESC_TYPE
     * field of the descriptor.
     */
    SQL_DATA_TYPE,

    /**
     * When the value of SQL_DATA_TYPE is SQL_DATETIME or SQL_INTERVAL,
     * this column contains the datetime/interval sub-code.
     */
    SQL_DATETIME_SUB,

    /**
     * If the data type is an approximate numeric type, this column
     * contains the value 2 to indicate that COLUMN_SIZE specifies a number
     * of bits.
     */
    NUM_PREC_RADIX,

    /**
     * If the data type is an interval data type, then this column contains
     * the value of the interval leading precision.
     */
    INTERVAL_PRECISION
  };
};
}  // namespace

namespace documentdb {
namespace odbc {
namespace query {
TypeInfoQuery::TypeInfoQuery(diagnostic::DiagnosableAdapter& diag,
                             Connection& connection,
                             int16_t sqlType)
    : Query(diag, QueryType::TYPE_INFO),
      connection(connection),
      requestedSqlType(sqlType),
      columnsMeta(),
      executed(false),
      fetched(false),
      binaryTypes(),
      cursor(binaryTypes.end()) {
  using namespace documentdb::odbc::impl::binary;
  using namespace documentdb::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(19);

  const std::string sch;
  const std::string tbl;

  columnsMeta.push_back(ColumnMeta(sch, tbl, "TYPE_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "DATA_TYPE", JDBC_TYPE_SMALLINT,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_SIZE", JDBC_TYPE_INTEGER,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "LITERAL_PREFIX",
                                   JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "LITERAL_SUFFIX",
                                   JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "CREATE_PARAMS", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NULLABLE", JDBC_TYPE_SMALLINT,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "CASE_SENSITIVE",
                                   JDBC_TYPE_SMALLINT,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "SEARCHABLE", JDBC_TYPE_SMALLINT,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "UNSIGNED_ATTRIBUTE",
                                   JDBC_TYPE_SMALLINT,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FIXED_PREC_SCALE",
                                   JDBC_TYPE_SMALLINT,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "AUTO_UNIQUE_VALUE",
                                   JDBC_TYPE_SMALLINT,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "LOCAL_TYPE_NAME",
                                   JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "MINIMUM_SCALE",
                                   JDBC_TYPE_SMALLINT,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "MAXIMUM_SCALE",
                                   JDBC_TYPE_SMALLINT,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "SQL_DATA_TYPE",
                                   JDBC_TYPE_SMALLINT,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "SQL_DATETIME_SUB",
                                   JDBC_TYPE_SMALLINT,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NUM_PREC_RADIX",
                                   JDBC_TYPE_INTEGER,
                                   Nullability::NULLABILITY_UNKNOWN));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "INTERVAL_PRECISION",
                                   JDBC_TYPE_SMALLINT,
                                   Nullability::NULLABILITY_UNKNOWN));

  assert(IsSqlTypeSupported(sqlType) || sqlType == SQL_ALL_TYPES);

}

TypeInfoQuery::~TypeInfoQuery() {
  // No-op.
}

SqlResult::Type TypeInfoQuery::Execute() {
  if (binaryTypes.size() == 0) {
    MakeRequestGetTypeInfo();
  }
  cursor = binaryTypes.begin();

  executed = true;
  fetched = false;

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type TypeInfoQuery::MakeRequestGetTypeInfo() {
  DocumentDbError error;
  SharedPointer< DatabaseMetaData > databaseMetaData =
      connection.GetMetaData(error);
  if (!databaseMetaData.IsValid()
      || error.GetCode() != DocumentDbError::DOCUMENTDB_SUCCESS) {
    diag.AddStatusRecord(error.GetText());
    return SqlResult::AI_ERROR;
  }

  JniErrorInfo errInfo;
  SharedPointer< ResultSet > resultSet =
      databaseMetaData.Get()->GetTypeInfo(errInfo);
  if (!resultSet.IsValid()
      || errInfo.code != JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS) {
    diag.AddStatusRecord(errInfo.errMsg);
    return SqlResult::AI_ERROR;
  }

  boost::optional< int32_t > optionalBinaryType;
  bool hasNext = false;
  JniErrorCode errCode;
  do {
    errCode = resultSet.Get()->Next(hasNext, errInfo);
    if (errCode != JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS) {
      diag.AddStatusRecord(errInfo.errMsg);
      return SqlResult::AI_ERROR;
    }
    if (!hasNext)
      break;
    errCode = resultSet.Get()->GetInt("DATA_TYPE", optionalBinaryType, errInfo);
    if (errCode != JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS) {
      diag.AddStatusRecord(errInfo.errMsg);
      return SqlResult::AI_ERROR;
    }

    int16_t currentBinaryType = static_cast< int16_t >(*optionalBinaryType);
    int16_t currentSqlType = *(type_traits::BinaryToSqlType(currentBinaryType));

    // Ignore duplicates
    if (sqlTypes.find(currentSqlType) != sqlTypes.end()) {
      continue;
    }

    if (requestedSqlType == SQL_ALL_TYPES) {
      binaryTypes.emplace(currentBinaryType);
      sqlTypes.emplace(currentSqlType);
    } else if (requestedSqlType == currentSqlType) {
      // Add first and only entry that satisfies requested SQL type.
      binaryTypes.emplace(currentBinaryType);
      sqlTypes.emplace(currentSqlType);
      break;
    }
  } while (hasNext);

  return SqlResult::AI_SUCCESS;
}

const meta::ColumnMetaVector* TypeInfoQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type TypeInfoQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (!fetched)
    fetched = true;
  else if (cursor != binaryTypes.end())
    ++cursor;
  if (cursor == binaryTypes.end())
    return SqlResult::AI_NO_DATA;

  app::ColumnBindingMap::iterator it;

  for (it = columnBindings.begin(); it != columnBindings.end(); ++it)
    GetColumn(it->first, it->second);

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type TypeInfoQuery::GetColumn(uint16_t columnIdx,
                                         app::ApplicationDataBuffer& buffer) {
  using namespace documentdb::odbc::impl::binary;

  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (cursor == binaryTypes.end()) {
    diag.AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE,
                         "Cursor has reached end of the result set.");

    return SqlResult::AI_ERROR;
  }

  int8_t currentType = *cursor;

  switch (columnIdx) {
    case ResultColumn::TYPE_NAME: {
      buffer.PutString(type_traits::BinaryTypeToSqlTypeName(currentType));

      break;
    }

    case ResultColumn::DATA_TYPE:
    case ResultColumn::SQL_DATA_TYPE: {
      buffer.PutInt16(type_traits::BinaryToSqlType(currentType));

      break;
    }

    case ResultColumn::COLUMN_SIZE: {
      buffer.PutInt32(type_traits::BinaryTypeColumnSize(currentType));

      break;
    }

    case ResultColumn::LITERAL_PREFIX: {
      switch (currentType) {
        case JDBC_TYPE_CHAR:
        case JDBC_TYPE_VARCHAR:
        case JDBC_TYPE_LONGVARCHAR:
        case JDBC_TYPE_NCHAR:
        case JDBC_TYPE_NVARCHAR:
        case JDBC_TYPE_LONGNVARCHAR:
          buffer.PutString("'");
          break;

        case JDBC_TYPE_BINARY:
        case JDBC_TYPE_VARBINARY:
        case JDBC_TYPE_LONGVARBINARY:
        case DOCUMENTDB_TYPE_BINARY:
          buffer.PutString("0x");
          break;

        default:
          buffer.PutNull();
          break;
      }

      break;
    }

    case ResultColumn::LITERAL_SUFFIX: {
      switch (currentType) {
        case JDBC_TYPE_CHAR:
        case JDBC_TYPE_VARCHAR:
        case JDBC_TYPE_LONGVARCHAR:
        case JDBC_TYPE_NCHAR:
        case JDBC_TYPE_NVARCHAR:
        case JDBC_TYPE_LONGNVARCHAR:
          buffer.PutString("'");
          break;

        default:
          buffer.PutNull();
          break;
      }

      break;
    }

    case ResultColumn::CREATE_PARAMS: {
      buffer.PutNull();

      break;
    }

    case ResultColumn::NULLABLE: {
      buffer.PutInt32(type_traits::BinaryTypeNullability(currentType));

      break;
    }

    case ResultColumn::CASE_SENSITIVE: {
      if (currentType == JDBC_TYPE_CHAR || currentType == JDBC_TYPE_VARCHAR
          || currentType == JDBC_TYPE_LONGVARCHAR
          || currentType == JDBC_TYPE_NCHAR || currentType == JDBC_TYPE_NVARCHAR
          || currentType == JDBC_TYPE_LONGNVARCHAR)
        buffer.PutInt16(SQL_TRUE);
      else
        buffer.PutInt16(SQL_FALSE);

      break;
    }

    case ResultColumn::SEARCHABLE: {
      buffer.PutInt16(SQL_SEARCHABLE);

      break;
    }

    case ResultColumn::UNSIGNED_ATTRIBUTE: {
      buffer.PutInt16(type_traits::BinaryTypeUnsigned(currentType));

      break;
    }

    case ResultColumn::FIXED_PREC_SCALE:
    case ResultColumn::AUTO_UNIQUE_VALUE: {
      buffer.PutInt16(SQL_FALSE);

      break;
    }

    case ResultColumn::LOCAL_TYPE_NAME: {
      buffer.PutNull();

      break;
    }

    case ResultColumn::MINIMUM_SCALE:
    case ResultColumn::MAXIMUM_SCALE: {
      buffer.PutInt16(type_traits::BinaryTypeDecimalDigits(currentType));

      break;
    }

    case ResultColumn::SQL_DATETIME_SUB: {
      buffer.PutNull();

      break;
    }

    case ResultColumn::NUM_PREC_RADIX: {
      buffer.PutInt32(type_traits::BinaryTypeNumPrecRadix(currentType));

      break;
    }

    case ResultColumn::INTERVAL_PRECISION: {
      buffer.PutNull();

      break;
    }

    default:
      break;
  }

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type TypeInfoQuery::Close() {
  cursor = binaryTypes.end();

  executed = false;

  return SqlResult::AI_SUCCESS;
}

bool TypeInfoQuery::DataAvailable() const {
  return cursor != binaryTypes.end();
}

int64_t TypeInfoQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type TypeInfoQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}
}  // namespace query
}  // namespace odbc
}  // namespace documentdb
