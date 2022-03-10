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

#include "ignite/odbc/query/column_metadata_query.h"

#include <vector>

#include "ignite/odbc/impl/binary/binary_common.h"
#include "ignite/odbc/common/concurrent.h"
#include "ignite/odbc/connection.h"
#include "ignite/odbc/ignite_error.h"
#include "ignite/odbc/jni/database_metadata.h"
#include "ignite/odbc/jni/java.h"
#include "ignite/odbc/jni/result_set.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/message.h"
#include "ignite/odbc/odbc_error.h"
#include "ignite/odbc/type_traits.h"

using ignite::odbc::IgniteError;
using ignite::odbc::common::concurrent::SharedPointer;
using ignite::odbc::jni::DatabaseMetaData;
using ignite::odbc::jni::java::JniErrorInfo;

namespace {
struct ResultColumn {
  enum Type {
    /** Catalog name. NULL if not applicable to the data source. */
    TABLE_CAT = 1,

    /** Schema name. NULL if not applicable to the data source. */
    TABLE_SCHEM,

    /** Table name. */
    TABLE_NAME,

    /** Column name. */
    COLUMN_NAME,

    /** SQL data type. */
    DATA_TYPE,

    /** Data source-dependent data type name. */
    TYPE_NAME,

    /** Column size. */
    COLUMN_SIZE,

    /** The length in bytes of data transferred on fetch. */
    BUFFER_LENGTH,

    /** The total number of significant digits to the right of the decimal
       point. */
    DECIMAL_DIGITS,

    /** Precision. */
    NUM_PREC_RADIX,

    /** Nullability of the data in column (int). */
    NULLABLE,

    /** A description of the column. */
    REMARKS,

    /** Default value for the column. May be null. */
    COLUMN_DEF,

    /** SQL data type. */
    SQL_DATA_TYPE,

    /** Subtype code for datetime and interval data types. */
    SQL_DATETIME_SUB,

    /** Maximum length in bytes of a character or binary data type column.
       NULL for other data types. */
    CHAR_OCTET_LENGTH,

    /** Index of column in table (starting at 1) */
    ORDINAL_POSITION,

    /** Nullability of data in column (String). */
    IS_NULLABLE
  };
};
}  // namespace

namespace ignite {
namespace odbc {
namespace query {
ColumnMetadataQuery::ColumnMetadataQuery(diagnostic::DiagnosableAdapter& diag,
                                         Connection& connection,
                                         const std::string& catalog,
                                         const std::string& schema,
                                         const std::string& table,
                                         const std::string& column)
    : Query(diag, QueryType::COLUMN_METADATA),
      connection(connection),
      catalog(catalog),
      schema(schema),
      table(table),
      column(column),
      executed(false),
      fetched(false),
      meta(),
      columnsMeta() {
  using namespace ignite::odbc::impl::binary;
  using namespace ignite::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(18);

  const std::string sch;
  const std::string tbl;

  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_CAT", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_SCHEM", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "DATA_TYPE", JDBC_TYPE_SMALLINT,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TYPE_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_SIZE", JDBC_TYPE_INTEGER,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "BUFFER_LENGTH", JDBC_TYPE_INTEGER,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "DECIMAL_DIGITS",
                                   JDBC_TYPE_SMALLINT, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NUM_PREC_RADIX",
                                   JDBC_TYPE_SMALLINT, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NULLABLE", JDBC_TYPE_SMALLINT,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "REMARKS", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_DEF", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "SQL_DATA_TYPE",
                                   JDBC_TYPE_SMALLINT, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "SQL_DATETIME_SUB",
                                   JDBC_TYPE_SMALLINT, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "CHAR_OCTET_LENGTH",
                                   JDBC_TYPE_INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "ORDINAL_POSITION",
                                   JDBC_TYPE_INTEGER, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "IS_NULLABLE", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
}

ColumnMetadataQuery::~ColumnMetadataQuery() {
  // No-op.
}

SqlResult::Type ColumnMetadataQuery::Execute() {
  if (executed)
    Close();

  SqlResult::Type result = MakeRequestGetColumnsMeta();

  if (result == SqlResult::AI_SUCCESS) {
    executed = true;
    fetched = false;

    cursor = meta.begin();
  }

  return result;
}

const meta::ColumnMetaVector* ColumnMetadataQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type ColumnMetadataQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (!fetched)
    fetched = true;
  else
    ++cursor;

  if (cursor == meta.end())
    return SqlResult::AI_NO_DATA;

  app::ColumnBindingMap::iterator it;

  for (it = columnBindings.begin(); it != columnBindings.end(); ++it)
    GetColumn(it->first, it->second);

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type ColumnMetadataQuery::GetColumn(
    uint16_t columnIdx, app::ApplicationDataBuffer& buffer) {
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (cursor == meta.end()) {
    diag.AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE,
                         "Cursor has reached end of the result set.");

    return SqlResult::AI_ERROR;
  }

  const meta::ColumnMeta& currentColumn = *cursor;
  int16_t columnType = currentColumn.GetDataType();

  switch (columnIdx) {
    case ResultColumn::TABLE_CAT: {
      buffer.PutString(currentColumn.GetCatalogName());
      break;
    }

    case ResultColumn::TABLE_SCHEM: {
      buffer.PutString(currentColumn.GetSchemaName());
      break;
    }

    case ResultColumn::TABLE_NAME: {
      buffer.PutString(currentColumn.GetTableName());
      break;
    }

    case ResultColumn::COLUMN_NAME: {
      buffer.PutString(currentColumn.GetColumnName());
      break;
    }

    case ResultColumn::DATA_TYPE: {
      buffer.PutInt16(type_traits::BinaryToSqlType(columnType));
      break;
    }

    case ResultColumn::TYPE_NAME: {
      buffer.PutString(
          type_traits::BinaryTypeToSqlTypeName(columnType));
      break;
    }

    case ResultColumn::COLUMN_SIZE: {
      buffer.PutInt32(type_traits::BinaryTypeColumnSize(columnType));
      break;
    }

    case ResultColumn::BUFFER_LENGTH: {
      buffer.PutInt32(type_traits::BinaryTypeTransferLength(columnType));
      break;
    }

    case ResultColumn::DECIMAL_DIGITS: {
      // todo implement the function for getting the decimal digits:
      // https://bitquill.atlassian.net/browse/AD-615
      int32_t decDigits = type_traits::BinaryTypeDecimalDigits(columnType);
      if (decDigits < 0)
        buffer.PutNull();
      else
        buffer.PutInt16(static_cast< int16_t >(decDigits));
      break;
    }

    case ResultColumn::NUM_PREC_RADIX: {
      int32_t numPrecRadix = type_traits::BinaryTypeNumPrecRadix(columnType);
      if (numPrecRadix < 0)
        buffer.PutNull();
      else
        buffer.PutInt16(static_cast< int16_t >(numPrecRadix));
      break;
    }

    case ResultColumn::NULLABLE: {
      buffer.PutInt16(currentColumn.GetNullability());
      break;
    }

    case ResultColumn::REMARKS: {
      buffer.PutString(currentColumn.GetRemarks());
      break;
    }

    case ResultColumn::COLUMN_DEF: {
      buffer.PutString(currentColumn.GetColumnDef());
      break;
    }

    case ResultColumn::SQL_DATA_TYPE: {
      buffer.PutInt16(type_traits::BinaryToSqlType(columnType));
      break;
    }

    case ResultColumn::SQL_DATETIME_SUB: {
      buffer.PutNull();
      // todo implement the function for getting the datetime sub code:
      // https://bitquill.atlassian.net/browse/AD-609
      break;
    }

    case ResultColumn::CHAR_OCTET_LENGTH: {
      buffer.PutInt32(type_traits::BinaryTypeCharOctetLength(columnType));
      break;
    }

    case ResultColumn::ORDINAL_POSITION: {
      buffer.PutInt32(currentColumn.GetOrdinalPosition());
      break;
    }

    case ResultColumn::IS_NULLABLE: {
      buffer.PutString(
          type_traits::NullabilityToIsNullable(currentColumn.GetNullability()));
      break;
    }

    default: {
      diag.AddStatusRecord(SqlState::S07009_INVALID_DESCRIPTOR_INDEX,
                           "Invalid index.");
      return SqlResult::AI_ERROR;
      break;
    }
  }

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type ColumnMetadataQuery::Close() {
  meta.clear();

  executed = false;

  return SqlResult::AI_SUCCESS;
}

bool ColumnMetadataQuery::DataAvailable() const {
  return cursor != meta.end();
}

int64_t ColumnMetadataQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type ColumnMetadataQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}

SqlResult::Type ColumnMetadataQuery::MakeRequestGetColumnsMeta() {
  IgniteError error;
  SharedPointer< DatabaseMetaData > databaseMetaData =
      connection.GetMetaData(error);
  if (!databaseMetaData.IsValid()
      || error.GetCode() != IgniteError::IGNITE_SUCCESS) {
    diag.AddStatusRecord(error.GetText());
    return SqlResult::AI_ERROR;
  }

  JniErrorInfo errInfo;
  SharedPointer< ResultSet > resultSet = databaseMetaData.Get()->GetColumns(
      catalog, schema, table, column, errInfo);
  if (!resultSet.IsValid()
      || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    diag.AddStatusRecord(errInfo.errMsg);
    return SqlResult::AI_ERROR;
  }

  meta::ReadColumnMetaVector(resultSet, meta);

  for (size_t i = 0; i < meta.size(); ++i) {
    LOG_MSG("\n[" << i << "] SchemaName:     " << meta[i].GetSchemaName()
                  << "\n[" << i
                  << "] TableName:      " << meta[i].GetTableName() << "\n["
                  << i << "] ColumnName:     " << meta[i].GetColumnName()
                  << "\n[" << i << "] ColumnType:     "
                  << static_cast< int32_t >(meta[i].GetDataType()));
  }

  return SqlResult::AI_SUCCESS;
}
}  // namespace query
}  // namespace odbc
}  // namespace ignite
