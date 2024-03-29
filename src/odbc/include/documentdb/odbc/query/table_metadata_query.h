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

#ifndef _DOCUMENTDB_ODBC_QUERY_TABLE_METADATA_QUERY
#define _DOCUMENTDB_ODBC_QUERY_TABLE_METADATA_QUERY

#include "documentdb/odbc/meta/table_meta.h"
#include "documentdb/odbc/query/query.h"

namespace documentdb {
namespace odbc {
/** Connection forward-declaration. */
class Connection;

namespace query {
/**
 * Query.
 */
class TableMetadataQuery : public Query {
 public:
  /**
   * Constructor.
   *
   * @param diag Diagnostics collector.
   * @param connection Associated connection.
   * @param catalog Catalog search pattern.
   * @param schema Schema search pattern.
   * @param table Table search pattern.
   * @param tableType Table type search pattern.
   */
  TableMetadataQuery(diagnostic::DiagnosableAdapter& diag,
                     Connection& connection,
                     const boost::optional< std::string >& catalog,
                     const boost::optional< std::string >& schema,
                     const std::string& table,
                     const boost::optional< std::string >& tableType);

  /**
   * Destructor.
   */
  virtual ~TableMetadataQuery();

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
  DOCUMENTDB_NO_COPY_ASSIGNMENT(TableMetadataQuery);

  /**
   * Make get columns metadata requets and use response to set internal state.
   *
   * @return True on success.
   */
  SqlResult::Type MakeRequestGetTablesMeta();

  /**
   * Trims leading space from a string.
   *
   * @return the string with leading spaces trimmed.
   */
  std::string ltrim(const std::string& s);

  /**
   * Trims trailing space from a string.
   *
   * @return the string with trailing spaces trimmed.
   */
  std::string rtrim(const std::string& s);

  /**
   * Trims leading and trailing space from a string.
   *
   * @return the string with leading and trailing spaces trimmed.
   */
  std::string trim(const std::string& s);

  /**
   * Remove outer matching quotes from a string. They can be either single (')
   * or double (") quotes. They must be the left- and right-most characters in
   * the string.
   *
   * @return the string with matched quotes removed.
   */
  std::string dequote(const std::string& s);

  /** Connection associated with the statement. */
  Connection& connection;

  /** Catalog search pattern. */
  boost::optional< std::string > catalog;

  /** Schema search pattern. */
  boost::optional< std::string > schema;

  /** Table search pattern. */
  std::string table;

  /** Table type search pattern. */
  boost::optional< std::string > tableType;

  /** Query executed. */
  bool executed;

  /** Fetched flag. */
  bool fetched;

  /** Fetched metadata. */
  meta::TableMetaVector meta;

  /** Metadata cursor. */
  meta::TableMetaVector::iterator cursor;

  /** Columns metadata. */
  meta::ColumnMetaVector columnsMeta;
};
}  // namespace query
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_QUERY_TABLE_METADATA_QUERY
