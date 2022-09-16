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

#ifndef _DOCUMENTDB_ODBC_QUERY_TYPE_INFO_QUERY
#define _DOCUMENTDB_ODBC_QUERY_TYPE_INFO_QUERY

#include "documentdb/odbc/query/query.h"
#include <set>

namespace documentdb {
namespace odbc {
namespace query {
/**
 * Type info query.
 */
class TypeInfoQuery : public Query {
 public:
  /**
   * Constructor.
   *
   * @param diag Diagnostics collector.
   * @param connection open connection.
   * @param sqlType SQL type.
   */
  TypeInfoQuery(diagnostic::DiagnosableAdapter& diag, Connection& connection, int16_t sqlType);

  /**
   * Destructor.
   */
  virtual ~TypeInfoQuery();

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
  DOCUMENTDB_NO_COPY_ASSIGNMENT(TypeInfoQuery);

  /** 
   * Makes request via the JNI interface and builds the types.
   */
  SqlResult::Type MakeRequestGetTypeInfo();

  /** Connection. */
  Connection& connection;

  /** Requested SQL type(s) */
  int16_t requestedSqlType;

  /** Columns metadata. */
  meta::ColumnMetaVector columnsMeta;

  /** Executed flag. */
  bool executed;

  /** Fetched flag. */
  bool fetched;

  /** Supported binary types. */
  std::set< int16_t > binaryTypes;

  /** Supported SQL types. */
  std::set< int16_t > sqlTypes;

  /** Query cursor. */
  std::set< int16_t >::const_iterator cursor;
};
}  // namespace query
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_QUERY_TYPE_INFO_QUERY
