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

#ifndef _IGNITE_ODBC_QUERY_DATA_QUERY
#define _IGNITE_ODBC_QUERY_DATA_QUERY

#include "ignite/odbc/app/parameter_set.h"
#include "ignite/odbc/documentdb_cursor.h"
#include "ignite/odbc/query/query.h"
#include "ignite/odbc/jni/documentdb_mql_query_context.h"

using ignite::odbc::jni::DocumentDbMqlQueryContext;

namespace ignite {
namespace odbc {
/** Connection forward-declaration. */
class Connection;

namespace query {
/**
 * Query.
 */
class DataQuery : public Query {
 public:
  /**
   * Constructor.
   *
   * @param diag Diagnostics collector.
   * @param connection Associated connection.
   * @param sql SQL query string.
   * @param params SQL params.
   * @param timeout Timeout.
   */
  DataQuery(diagnostic::DiagnosableAdapter& diag, Connection& connection,
            const std::string& sql, const app::ParameterSet& params,
            int32_t& timeout);

  /**
   * Destructor.
   */
  virtual ~DataQuery();

  /**
   * Execute query.
   *
   * @return True on success.
   */
  virtual SqlResult::Type Execute();

  /**
   * Get column metadata.
   *
   * @return Column metadata.
   */
  virtual const meta::ColumnMetaVector* GetMeta();

  /**
   * Fetch next result row to application buffers.
   *
   * @param columnBindings Application buffers to put data to.
   * @return Operation result.
   */
  virtual SqlResult::Type FetchNextRow(app::ColumnBindingMap& columnBindings);

  /**
   * Get data of the specified column in the result set.
   *
   * @param columnIdx Column index.
   * @param buffer Buffer to put column data to.
   * @return Operation result.
   */
  virtual SqlResult::Type GetColumn(uint16_t columnIdx,
                                    app::ApplicationDataBuffer& buffer);

  /**
   * Close query.
   *
   * @return Result.
   */
  virtual SqlResult::Type Close();

  /**
   * Check if data is available.
   *
   * @return True if data is available.
   */
  virtual bool DataAvailable() const;

  /**
   * Get number of rows affected by the statement.
   *
   * @return Number of rows affected by the statement.
   */
  virtual int64_t AffectedRows() const;

  /**
   * Move to the next result set.
   *
   * @return Operaion result.
   */
  virtual SqlResult::Type NextResultSet();

  /**
   * Get SQL query string.
   *
   * @return SQL query string.
   */
  const std::string& GetSql() const {
    return sql_;
  }

 private:
  IGNITE_NO_COPY_ASSIGNMENT(DataQuery);

  /**
   * Make query prepare request and use response to set internal
   * state.
   *
   * @return Result.
   */
  SqlResult::Type MakeRequestPrepare();

  /**
   * Make query execute request and use response to set internal
   * state.
   *
   * @return Result.
   */
  SqlResult::Type MakeRequestExecute();

  /**
   * Make query close request.
   *
   * @return Result.
   */
  SqlResult::Type MakeRequestClose();

  /**
   * Make data fetch request and use response to set internal state.
   *
   * @return Result.
   */
  SqlResult::Type MakeRequestFetch();


  /**
   * Gets the MQL query context.
   *
   * @return Result.
   */
  SqlResult::Type GetMqlQueryContext(
      SharedPointer< DocumentDbMqlQueryContext >& mqlQueryContext,
      IgniteError& error);

  /**
   * Make next result set request and use response to set internal state.
   *
   * @return Result.
   */
  SqlResult::Type MakeRequestMoreResults();

  /**
   * Make result set metadata request.
   *
   * @return Result.
   */
  SqlResult::Type MakeRequestResultsetMeta();

  /**
   * Process column conversion operation result.
   *
   * @param convRes Conversion result.
   * @param rowIdx Row index.
   * @param columnIdx Column index.
   * @return General SQL result.
   */
  SqlResult::Type ProcessConversionResult(app::ConversionResult::Type convRes,
                                          int32_t rowIdx, int32_t columnIdx);
  ;

  /**
   * Set result set meta. 
   *
   * @param value Metadata value.
   */
  void SetResultsetMeta(const meta::ColumnMetaVector& value);


  /**
   * Set result set meta by reading Jdbc column metadata vector.
   *
   * @param jdbcMetaVector JdbcColumnMetadata vector.
   */
  void ReadJdbcColumnMetadataVector(
      std::vector< JdbcColumnMetadata > jdbcVector);

  /**
   * Close query.
   *
   * @return Result.
   */
  SqlResult::Type InternalClose();

  /** Connection associated with the statement. */
  Connection& connection_;

  /** SQL Query. */
  std::string sql_;

  /** Parameter bindings. */
  const app::ParameterSet& params_;

  /** Result set metadata is available */
  bool resultMetaAvailable_ = false;

  /** Result set metadata. */
  meta::ColumnMetaVector resultMeta_{};

  /** Cursor. */
  std::unique_ptr< DocumentDbCursor > cursor_{};

  /** Timeout. */
  int32_t& timeout_;
};
}  // namespace query
}  // namespace odbc
}  // namespace ignite

#endif  //_IGNITE_ODBC_QUERY_DATA_QUERY
