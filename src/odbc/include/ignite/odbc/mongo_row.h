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

#ifndef _IGNITE_ODBC_MONGO_ROW
#define _IGNITE_ODBC_MONGO_ROW

#include <stdint.h>

#include <vector>

#include "ignite/odbc/app/application_data_buffer.h"
#include "ignite/odbc/jni/jdbc_column_metadata.h"
#include "ignite/odbc/mongo_column.h"
#include "bsoncxx/document/view.hpp"
#include "mongocxx/cursor.hpp"

using namespace ignite::odbc::impl::interop;
using namespace ignite::odbc::impl::binary;
using ignite::odbc::jni::JdbcColumnMetadata;

namespace ignite {
namespace odbc {
/**
 * Query result row.
 */
class MongoRow {
 public:
  /**
   * Constructor.
   *
   * @param pageData Page data.
   */
  MongoRow(bsoncxx::document::view const& document,
           std::vector< JdbcColumnMetadata >& columnMetadata,
           std::vector< std::string >& paths);

  /**
   * Destructor.
   */
  ~MongoRow();

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
      uint16_t columnIdx, app::ApplicationDataBuffer& dataBuf);

 private:
  IGNITE_NO_COPY_ASSIGNMENT(MongoRow);

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
  MongoColumn& GetColumn(uint16_t columnIdx) {
    return columns_[columnIdx - 1];
  }

  /**
   * Ensure that column data is discovered.
   *
   * @param columnIdx Column index.
   * @return True if the column is discovered and false if it can not
   * be discovered.
   */
  bool EnsureColumnDiscovered(int16_t columnIdx);

  /** Current position in row. */
  int32_t pos;

  /** Row size in columns. */
  int32_t size;

  /** Columns. */
  std::vector< MongoColumn > columns_;

  /** The current document */
  bsoncxx::document::view document_;

  /** The column metadata */
  std::vector< JdbcColumnMetadata >& columnMetadata_;

  /** The matching paths in the document for the columns */
  std::vector< std::string >& paths_;
};
}  // namespace odbc
}  // namespace ignite

#endif  //_IGNITE_ODBC_MONGO_ROW
