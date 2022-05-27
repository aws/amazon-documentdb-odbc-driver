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

#ifndef _IGNITE_ODBC_QUERY_FOREIGN_KEYS_QUERY
#define _IGNITE_ODBC_QUERY_FOREIGN_KEYS_QUERY

#include "ignite/odbc/connection.h"
#include "ignite/odbc/query/query.h"
#include "ignite/odbc/meta/foreign_key_meta.h"

namespace ignite {
namespace odbc {
namespace query {
/**
 * Foreign keys query.
 */
class ForeignKeysQuery : public Query {
 public:

  /**
   * Constructor.
   *
   * @param diag Diagnostics collector.
   * @param connection Statement-associated connection.
   * @param catalog Foreign key catalog name.
   * @param schema Foreign key schema name.
   * @param table Foreign key table name.
   */
  ForeignKeysQuery(diagnostic::DiagnosableAdapter& diag, Connection& connection,
                   const boost::optional< std::string >& catalog,
                   const boost::optional< std::string >& schema,
                   const std::string& table);

  /**
   * Destructor.
   */
  virtual ~ForeignKeysQuery();

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
  IGNITE_NO_COPY_ASSIGNMENT(ForeignKeysQuery);

  /**
   * Make get foreign keys metadata requets and use response to set internal
   * state.
   *
   * @return Operation result.
   */
  virtual SqlResult::Type MakeRequestGetForeignKeysMeta();

  /** Connection associated with the statement. */
  Connection& connection;

  /** Foreign key catalog name. */
  const boost::optional< std::string > catalog;

  /** Foreign key schema name. */
  const boost::optional< std::string > schema;

  /** Foreign key table name. */
  std::string table;

  /** Query executed. */
  bool executed;

  /** Fetched flag. */
  bool fetched;

  /** Columns metadata. */
  meta::ColumnMetaVector columnsMeta;

  /** Metadata cursor. */
  meta::ForeignKeyMetaVector::iterator cursor;

  /** Fetched Foreign Keys metadata. */
  meta::ForeignKeyMetaVector meta;
};
}  // namespace query
}  // namespace odbc
}  // namespace ignite

#endif  //_IGNITE_ODBC_QUERY_FOREIGN_KEYS_QUERY