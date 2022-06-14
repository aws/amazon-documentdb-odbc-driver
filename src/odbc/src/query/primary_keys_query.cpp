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

#include "ignite/odbc/query/primary_keys_query.h"

#include "ignite/odbc/connection.h"
#include "ignite/odbc/impl/binary/binary_common.h"
#include "ignite/odbc/message.h"
#include "ignite/odbc/type_traits.h"

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

    /** Column sequence number in key. */
    KEY_SEQ,

    /** Primary key name. */
    PK_NAME
  };
};
}  // namespace

namespace ignite {
namespace odbc {
namespace query {
PrimaryKeysQuery::PrimaryKeysQuery(diagnostic::DiagnosableAdapter& diag,
                                   Connection& connection,
                                   const boost::optional< std::string >& catalog,
                                   const boost::optional< std::string >& schema,
                                   const boost::optional< std::string >& table)
    : Query(diag, QueryType::PRIMARY_KEYS),
      connection(connection),
      catalog(catalog),
      schema(schema),
      table(table),
      executed(false),
      columnsMeta() {
  using namespace ignite::odbc::impl::binary;
  using namespace ignite::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(6);

  const std::string sch("");
  const std::string tbl("");

  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_CAT", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_SCHEM", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "KEY_SEQ", JDBC_TYPE_SMALLINT,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PK_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
}

PrimaryKeysQuery::~PrimaryKeysQuery() {
  // No-op.
}

SqlResult::Type PrimaryKeysQuery::Execute() {
  if (executed)
    Close();

  SqlResult::Type result = MakeRequestGetPrimaryKeysMeta();

  if (result == SqlResult::AI_SUCCESS) {
    executed = true;
    fetched = false;

    cursor = meta.begin();
  }

  return result;
}

const meta::ColumnMetaVector* PrimaryKeysQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type PrimaryKeysQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  // An extra if check is needed here to prevent iterating past vector end
  if (cursor == meta.end())
    return SqlResult::AI_NO_DATA;

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

SqlResult::Type PrimaryKeysQuery::GetColumn(
    uint16_t columnIdx, app::ApplicationDataBuffer& buffer) {
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (cursor == meta.end())
    return SqlResult::AI_NO_DATA;

  const meta::PrimaryKeyMeta& currentColumn = *cursor;

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

    case ResultColumn::KEY_SEQ: {
      buffer.PutInt16(currentColumn.GetKeySeq());
      break;
    }

    case ResultColumn::PK_NAME: {
      buffer.PutString(currentColumn.GetKeyName());
      break;
    }

    default:
      break;
  }

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type PrimaryKeysQuery::Close() {
  meta.clear();

  executed = false;
  fetched = false;

  return SqlResult::AI_SUCCESS;
}

bool PrimaryKeysQuery::DataAvailable() const {
  return cursor != meta.end();
}

int64_t PrimaryKeysQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type PrimaryKeysQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}

SqlResult::Type PrimaryKeysQuery::MakeRequestGetPrimaryKeysMeta() {
  IgniteError error;
  SharedPointer< DatabaseMetaData > databaseMetaData =
      connection.GetMetaData(error);
  if (!databaseMetaData.IsValid()
      || error.GetCode() != IgniteError::IGNITE_SUCCESS) {
    diag.AddStatusRecord(error.GetText());
    return SqlResult::AI_ERROR;
  }

  JniErrorInfo errInfo;
  SharedPointer< ResultSet > resultSet =
      databaseMetaData.Get()->GetPrimaryKeys(catalog, schema, table, errInfo);
  if (!resultSet.IsValid()
      || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    diag.AddStatusRecord(errInfo.errMsg);
    return SqlResult::AI_ERROR;
  }

  meta::ReadPrimaryKeysColumnMetaVector(resultSet, meta);

  for (size_t i = 0; i < meta.size(); ++i) {
    LOG_DEBUG_MSG("\n[" << i << "] SchemaName:     "
                       << meta[i].GetSchemaName().get_value_or("") << "\n[" << i
                       << "] TableName:      "
                       << meta[i].GetTableName().get_value_or("") << "\n[" << i
                       << "] ColumnName:     "
                       << meta[i].GetColumnName().get_value_or("") << "\n[" << i
                       << "] ColumnType: not available");
  }
  return SqlResult::AI_SUCCESS;
}
}  // namespace query
}  // namespace odbc
}  // namespace ignite
