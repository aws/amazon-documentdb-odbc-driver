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

#include "ignite/odbc/query/foreign_keys_query.h"

#include "ignite/odbc/connection.h"
#include "ignite/odbc/impl/binary/binary_common.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/message.h"
#include "ignite/odbc/type_traits.h"

namespace {
struct ResultColumn {
  enum Type {
    /** Primary key table catalog being imported. NULL if not applicable to the
       data source. */
    PKTABLE_CAT = 1,

    /** Primary key table schema being imported. NULL if not applicable to the
       data source. */
    PKTABLE_SCHEM,

    /** Primary key table name being imported. */
    PKTABLE_NAME,

    /** Primary key column name being imported. */
    PKCOLUMN_NAME,

    /** Foreign key table catalog being imported. NULL if not applicable to the
       data source. */
    FKTABLE_CAT,

    /** Foreign key table schema being imported. NULL if not applicable to the
       data source. */
    FKTABLE_SCHEM,

    /** Foreign key table name being imported. */
    FKTABLE_NAME,

    /** Foreign key column name being imported. */
    FKCOLUMN_NAME,

    /** Sequence number within a foreign key
     * (a value of 1 represents the first column of the foreign key, a value of
     * 2 would represent the second column within the foreign key). */
    KEY_SEQ,

    /** Rule for updating a foreign key when the primary key is updated. */
    UPDATE_RULE,

    /** Rule for updating a foreign key when the primary key is deleted. */
    DELETE_RULE,

    /** Foreign key name. */
    FK_NAME,

    /** Primary key name. */
    PK_NAME,

    /** Deferrability of foreign key */
    DEFERRABILITY
  };
};
}  // namespace

namespace ignite {
namespace odbc {
namespace query {
ForeignKeysQuery::ForeignKeysQuery(diagnostic::DiagnosableAdapter& diag,
                                   Connection& connection,
                                   const boost::optional< std::string >& catalog,
                                   const boost::optional< std::string >& schema,
                                   const std::string& table)
    : Query(diag, QueryType::FOREIGN_KEYS),
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

  columnsMeta.reserve(14);

  const std::string sch("");
  const std::string tbl("");

  columnsMeta.push_back(ColumnMeta(sch, tbl, "PKTABLE_CAT", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PKTABLE_SCHEM", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PKTABLE_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PKCOLUMN_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FKTABLE_CAT", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FKTABLE_SCHEM", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FKTABLE_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FKCOLUMN_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "KEY_SEQ", JDBC_TYPE_SMALLINT,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "UPDATE_RULE", JDBC_TYPE_SMALLINT,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "DELETE_RULE", JDBC_TYPE_SMALLINT,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FK_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PK_NAME", JDBC_TYPE_VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "DEFERRABILITY",
                                   JDBC_TYPE_SMALLINT, Nullability::NO_NULL));
}

ForeignKeysQuery::~ForeignKeysQuery() {
  // No-op.
}

SqlResult::Type ForeignKeysQuery::Execute() {
  if (executed)
    Close();

  SqlResult::Type result = MakeRequestGetForeignKeysMeta();

  if (result == SqlResult::AI_SUCCESS) {
    executed = true;
    fetched = false;

    cursor = meta.begin();
  }

  return result;
}

const meta::ColumnMetaVector* ForeignKeysQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type ForeignKeysQuery::FetchNextRow(
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

SqlResult::Type ForeignKeysQuery::GetColumn(
    uint16_t columnIdx, app::ApplicationDataBuffer& buffer) {
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (cursor == meta.end())
    return SqlResult::AI_NO_DATA;

  const meta::ForeignKeyMeta& currentColumn = *cursor;

  switch (columnIdx) {
    case ResultColumn::PKTABLE_CAT: {
      buffer.PutString(currentColumn.GetPKCatalogName());
      break;
    }

    case ResultColumn::PKTABLE_SCHEM: {
      buffer.PutString(currentColumn.GetPKSchemaName());
      break;
    }

    case ResultColumn::PKTABLE_NAME: {
      buffer.PutString(currentColumn.GetPKTableName());
      break;
    }

    case ResultColumn::PKCOLUMN_NAME: {
      buffer.PutString(currentColumn.GetPKColumnName());
      break;
    }

    case ResultColumn::FKTABLE_CAT: {
      buffer.PutString(currentColumn.GetFKCatalogName());
      break;
    }

    case ResultColumn::FKTABLE_SCHEM: {
      buffer.PutString(currentColumn.GetFKSchemaName());
      break;
    }

    case ResultColumn::FKTABLE_NAME: {
      buffer.PutString(currentColumn.GetFKTableName());
      break;
    }

    case ResultColumn::FKCOLUMN_NAME: {
      buffer.PutString(currentColumn.GetFKColumnName());
      break;
    }

    case ResultColumn::KEY_SEQ: {
      buffer.PutInt16(currentColumn.GetKeySeq());
      break;
    }

    case ResultColumn::UPDATE_RULE: {
      buffer.PutInt16(currentColumn.GetUpdateRule());
      break;
    }

    case ResultColumn::DELETE_RULE: {
      buffer.PutInt16(currentColumn.GetDeleteRule());
      break;
    }

    case ResultColumn::FK_NAME: {
      buffer.PutString(currentColumn.GetFKName());
      break;
    }

    case ResultColumn::PK_NAME: {
      buffer.PutString(currentColumn.GetPKName());
      break;
    }

    case ResultColumn::DEFERRABILITY: {
      buffer.PutInt16(currentColumn.GetDeferrability());
      break;
    }

    default:
      break;
  }

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type ForeignKeysQuery::Close() {
  meta.clear();

  executed = false;
  fetched = false;

  return SqlResult::AI_SUCCESS;
}

bool ForeignKeysQuery::DataAvailable() const {
  return cursor != meta.end();
}
int64_t ForeignKeysQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type ForeignKeysQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}

SqlResult::Type ForeignKeysQuery::MakeRequestGetForeignKeysMeta() {
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
      databaseMetaData.Get()->GetImportedKeys(catalog, schema, table, errInfo);
  if (!resultSet.IsValid()
      || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    diag.AddStatusRecord(errInfo.errMsg);
    return SqlResult::AI_ERROR;
  }

  meta::ReadForeignKeysColumnMetaVector(resultSet, meta);

  for (size_t i = 0; i < meta.size(); ++i) {
    LOG_DEBUG_MSG("\n[" << i << "] PKSchemaName:     "
                        << meta[i].GetPKSchemaName().get_value_or("") << "\n["
                        << i << "] PKTableName:      "
                        << meta[i].GetPKTableName().get_value_or("") << "\n["
                        << i << "] PKColumnName:     "
                        << meta[i].GetPKColumnName().get_value_or("") << "\n["
                        << i << "] KeySeq:           " 
                        << meta[i].GetKeySeq());
  }

  return SqlResult::AI_SUCCESS;
}
}  // namespace query
}  // namespace odbc
}  // namespace ignite
