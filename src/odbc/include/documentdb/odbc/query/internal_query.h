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

#ifndef _DOCUMENTDB_ODBC_QUERY_INTERNAL_QUERY
#define _DOCUMENTDB_ODBC_QUERY_INTERNAL_QUERY

#include <stdint.h>

#include <map>

#include "documentdb/odbc/common_types.h"
#include "documentdb/odbc/diagnostic/diagnosable.h"
#include "documentdb/odbc/meta/column_meta.h"
#include "documentdb/odbc/query/query.h"
#include "documentdb/odbc/sql/sql_command.h"

namespace documentdb {
namespace odbc {
namespace query {
/**
 * Query.
 */
class InternalQuery : public Query {
 public:
  /**
   * Constructor.
   *
   * @param diag Diagnosable.
   * @param sql SQL query.
   * @param cmd Parsed command.
   */
  InternalQuery(diagnostic::DiagnosableAdapter& diag, const std::string& sql,
                std::shared_ptr< SqlCommand > cmd)
      : Query(diag, QueryType::INTERNAL), sql(sql), cmd(cmd) {
    // No-op.
  }

  /**
   * Destructor.
   */
  virtual ~InternalQuery() {
    // No-op.
  }

  /**
   * Execute query.
   *
   * @return True on success.
   */
  virtual SqlResult::Type Execute() {
    diag.AddStatusRecord("Internal error.");

    return SqlResult::AI_ERROR;
  }

  /**
   * Fetch next result row to application buffers.
   *
   * @param columnBindings Application buffers to put data to.
   * @return Operation result.
   */
  virtual SqlResult::Type FetchNextRow(app::ColumnBindingMap& columnBindings) {
    DOCUMENTDB_UNUSED(columnBindings);

    return SqlResult::AI_NO_DATA;
  }

  /**
   * Get data of the specified column in the result set.
   *
   * @param columnIdx Column index.
   * @param buffer Buffer to put column data to.
   * @return Operation result.
   */
  virtual SqlResult::Type GetColumn(uint16_t columnIdx,
                                    app::ApplicationDataBuffer& buffer) {
    DOCUMENTDB_UNUSED(columnIdx);
    DOCUMENTDB_UNUSED(buffer);

    return SqlResult::AI_NO_DATA;
  }

  /**
   * Close query.
   *
   * @return Operation result.
   */
  virtual SqlResult::Type Close() {
    return SqlResult::AI_SUCCESS;
  }

  /**
   * Get column metadata.
   *
   * @return Column metadata.
   */
  virtual const meta::ColumnMetaVector* GetMeta() {
    return 0;
  }

  /**
   * Check if data is available.
   *
   * @return True if data is available.
   */
  virtual bool DataAvailable() const {
    return false;
  }

  /**
   * Get number of rows affected by the statement.
   *
   * @return Number of rows affected by the statement.
   */
  virtual int64_t AffectedRows() const {
    return 0;
  }

  /**
   * Move to the next result set.
   *
   * @return Operation result.
   */
  virtual SqlResult::Type NextResultSet() {
    return SqlResult::AI_NO_DATA;
  }

  /**
   * Get SQL query.
   *
   * @return SQL query.
   */
  SqlCommand& GetCommand() const {
    return *cmd;
  }

  /**
   * Get SQL query.
   *
   * @return SQL Query.
   */
  const std::string& GetQuery() const {
    return sql;
  }

 protected:
  /** SQL string*/
  std::string sql;

  /** SQL command. */
  std::shared_ptr< SqlCommand > cmd;
};
}  // namespace query
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_QUERY_INTERNAL_QUERY
