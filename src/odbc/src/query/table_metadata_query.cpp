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

#include "ignite/odbc/query/table_metadata_query.h"

#include <regex>
#include <vector>

#include "ignite/odbc/common/concurrent.h"
#include "ignite/odbc/connection.h"
#include "ignite/odbc/ignite_error.h"
#include "ignite/odbc/impl/binary/binary_common.h"
#include "ignite/odbc/jni/database_metadata.h"
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

    /** Table type. */
    TABLE_TYPE,

    /** A description of the column. */
    REMARKS
  };
};
}  // namespace

namespace ignite {
namespace odbc {
namespace query {
TableMetadataQuery::TableMetadataQuery(diagnostic::DiagnosableAdapter& diag,
                                       Connection& connection,
                                       const std::string& catalog,
                                       const std::string& schema,
                                       const std::string& table,
                                       const std::string& tableType)
    : Query(diag, QueryType::TABLE_METADATA),
      connection(connection),
      catalog(catalog),
      schema(schema),
      table(table),
      tableType(tableType),
      executed(false),
      fetched(false),
      meta(),
      columnsMeta() {
  using namespace ignite::odbc::impl::binary;
  using namespace ignite::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(5);

  const std::string sch("");
  const std::string tbl("");

  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_CAT", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_SCHEM", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_TYPE", JDBC_TYPE_VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "REMARKS", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
}

TableMetadataQuery::~TableMetadataQuery() {
  // No-op.
}

SqlResult::Type TableMetadataQuery::Execute() {
  if (executed)
    Close();

  SqlResult::Type result = MakeRequestGetTablesMeta();

  if (result == SqlResult::AI_SUCCESS) {
    executed = true;
    fetched = false;

    cursor = meta.begin();
  }

  return result;
}

const meta::ColumnMetaVector* TableMetadataQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type TableMetadataQuery::FetchNextRow(
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

SqlResult::Type TableMetadataQuery::GetColumn(
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

  const meta::TableMeta& currentColumn = *cursor;

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

    case ResultColumn::TABLE_TYPE: {
      buffer.PutString(currentColumn.GetTableType());
      break;
    }

    case ResultColumn::REMARKS: {
      buffer.PutString(currentColumn.GetRemarks());
      break;
    }

    default:
      break;
  }

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type TableMetadataQuery::Close() {
  meta.clear();

  executed = false;

  return SqlResult::AI_SUCCESS;
}

bool TableMetadataQuery::DataAvailable() const {
  return cursor != meta.end();
}

int64_t TableMetadataQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type TableMetadataQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}

SqlResult::Type TableMetadataQuery::MakeRequestGetTablesMeta() {
  IgniteError error;
  SharedPointer< DatabaseMetaData > databaseMetaData =
      connection.GetMetaData(error);
  if (!databaseMetaData.IsValid()
      || error.GetCode() != IgniteError::IGNITE_SUCCESS) {
    diag.AddStatusRecord(error.GetText());
    return SqlResult::AI_ERROR;
  }

  std::vector< std::string > types;
  if (tableType.empty()) {
    types.emplace_back("TABLE");
  } else {
    std::stringstream ss(tableType);
    std::string singleTableType;
    while (std::getline(ss, singleTableType, ',')) {
      types.push_back(dequote(trim(singleTableType)));
    }
  }

  JniErrorInfo errInfo;
  SharedPointer< ResultSet > resultSet =
      databaseMetaData.Get()->GetTables(catalog, schema, table, types, errInfo);
  if (!resultSet.IsValid()
      || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    diag.AddStatusRecord(errInfo.errMsg);
    return SqlResult::AI_ERROR;
  }

  meta::ReadTableMetaVector(resultSet, meta);

  for (size_t i = 0; i < meta.size(); ++i) {
    LOG_MSG("\n[" << i << "] CatalogName: " << meta[i].GetCatalogName() << "\n["
                  << i << "] SchemaName:  " << meta[i].GetSchemaName() << "\n["
                  << i << "] TableName:   " << meta[i].GetTableName() << "\n["
                  << i << "] TableType:   " << meta[i].GetTableType());
  }

  return SqlResult::AI_SUCCESS;
}

std::string TableMetadataQuery::ltrim(const std::string& s) {
  return std::regex_replace(s, std::regex("^\\s+"), std::string(""));
}

std::string TableMetadataQuery::rtrim(const std::string& s) {
  return std::regex_replace(s, std::regex("\\s+$"), std::string(""));
}

std::string TableMetadataQuery::trim(const std::string& s) {
  return ltrim(rtrim(s));
}

std::string TableMetadataQuery::dequote(const std::string& s) {
  if (s.size() >= 2
      && ((s.front() == '\'' && s.back() == '\'')
          || (s.front() == '"' && s.back() == '"'))) {
    return s.substr(1, s.size() - 2);
  }
  return s;
}
}  // namespace query
}  // namespace odbc
}  // namespace ignite
