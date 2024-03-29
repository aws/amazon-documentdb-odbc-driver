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

#ifndef _DOCUMENTDB_ODBC_DOCUMENTDB_ROW
#define _DOCUMENTDB_ODBC_DOCUMENTDB_ROW

#include <stdint.h>

#include <vector>

#include "documentdb/odbc/app/application_data_buffer.h"
#include "documentdb/odbc/jni/jdbc_column_metadata.h"
#include "documentdb/odbc/documentdb_column.h"
#include "bsoncxx/document/view.hpp"
#include "mongocxx/cursor.hpp"

using namespace documentdb::odbc::impl::interop;
using namespace documentdb::odbc::impl::binary;
using documentdb::odbc::jni::JdbcColumnMetadata;

namespace documentdb {
namespace odbc {
/**
 * Query result row.
 */
class DocumentDbRow {
 public:
  /**
   * Constructor.
   *
   * @param pageData Page data.
   */
  DocumentDbRow(bsoncxx::document::view const& document,
                std::vector< JdbcColumnMetadata >& columnMetadata,
                std::vector< std::string >& paths);

  /**
   * Destructor.
   */
  ~DocumentDbRow() = default;

  /**
   * Get row size in columns.
   *
   * @return number of columns.
   */
  int32_t GetSize() const {
    return columnMetadata_.size();
  }

  /**
   * Read column data and store it in application data buffer.
   *
   * @param columnIdx Column index.
   * @param dataBuf Application data buffer.
   * @return Conversion result.
   */
  app::ConversionResult::Type ReadColumnToBuffer(
      uint32_t columnIdx, app::ApplicationDataBuffer& dataBuf);

  /**
   * Updates the row and columns with a new document.
   */
  void Update(bsoncxx::document::view const& document);

 private:
  DOCUMENTDB_NO_COPY_ASSIGNMENT(DocumentDbRow);

  /**
   * Get columns by its index.
   *
   * Column indexing starts at 1.
   *
   * @note This operation is private because it's unsafe to use:
   *       It is neccessary to ensure that column is discovered prior
   *       to calling this method using EnsureColumnDiscovered().
   *
   * @param columnIdx Column index.
   * @return Reference to specified column.
   */
  DocumentDbColumn& GetColumn(uint32_t columnIdx) {
    return columns_[columnIdx - 1];
  }

  /**
   * Ensure that column data is discovered.
   *
   * @param columnIdx Column index.
   * @return True if the column is discovered and false if it can not
   * be discovered.
   */
  bool EnsureColumnDiscovered(uint32_t columnIdx);

  /** Current position in row. */
  int32_t pos;

  /** Row size in columns. */
  int32_t size;

  /** Columns. */
  std::vector< DocumentDbColumn > columns_;

  /** The current document */
  bsoncxx::document::view document_;

  /** The column metadata */
  std::vector< JdbcColumnMetadata >& columnMetadata_;

  /** The matching paths in the document for the columns */
  std::vector< std::string >& paths_;
};
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_DOCUMENTDB_ROW
