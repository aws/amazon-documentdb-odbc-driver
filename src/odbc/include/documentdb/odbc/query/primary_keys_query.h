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

#ifndef _DOCUMENTDB_ODBC_QUERY_PRIMARY_KEYS_QUERY
#define _DOCUMENTDB_ODBC_QUERY_PRIMARY_KEYS_QUERY

#include "documentdb/odbc/connection.h"
#include "documentdb/odbc/meta/primary_key_meta.h"
#include "documentdb/odbc/query/query.h"

namespace documentdb {
namespace odbc {
namespace query {
/**
 * Primary keys query.
 */
class PrimaryKeysQuery : public Query {
 public:
  /**
   * Constructor.
   *
   * @param diag Diagnostics collector.
   * @param connection Statement-associated connection.
   * @param catalog Catalog name.
   * @param schema Schema name.
   * @param table Table name.
   */
  PrimaryKeysQuery(diagnostic::DiagnosableAdapter& diag, Connection& connection,
                   const boost::optional< std::string >& catalog,
                   const boost::optional< std::string >& schema,
                   const boost::optional< std::string >& table);

  /**
   * Destructor.
   */
  virtual ~PrimaryKeysQuery();

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
   * @return True on success.
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
   * @return Operation result.
   */
  virtual SqlResult::Type NextResultSet();

 private:
  DOCUMENTDB_NO_COPY_ASSIGNMENT(PrimaryKeysQuery);

  /**
   * Make get primary keys metadata requets and use response to set internal
   * state.
   *
   * @return Operation result.
   */
  virtual SqlResult::Type MakeRequestGetPrimaryKeysMeta();

  /** Connection associated with the statement. */
  Connection& connection;

  /** Catalog name. */
  boost::optional< std::string > catalog;

  /** Schema name. */
  boost::optional< std::string > schema;

  /** Table name. */
  boost::optional< std::string > table;

  /** Query executed. */
  bool executed;

  /** Fetched flag. */
  bool fetched;

  /** Columns metadata. */
  meta::ColumnMetaVector columnsMeta;

  /** Primary keys metadata. */
  meta::PrimaryKeyMetaVector meta;

  /** Resultset cursor. */
  meta::PrimaryKeyMetaVector::iterator cursor;
};
}  // namespace query
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_QUERY_PRIMARY_KEYS_QUERY
