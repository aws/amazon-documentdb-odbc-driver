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

#ifndef _DOCUMENTDB_ODBC_DOCUMENTDB_CURSOR
#define _DOCUMENTDB_ODBC_DOCUMENTDB_CURSOR

#include <stdint.h>

#include <map>
#include <memory>

#include "documentdb/odbc/common_types.h"
#include "documentdb/odbc/result_page.h"
#include "documentdb/odbc/documentdb_row.h"
#include "mongocxx/client.hpp"
#include "mongocxx/cursor.hpp"

namespace documentdb {
namespace odbc {
/**
 * Query result cursor.
 */
class DocumentDbCursor {
 public:
  /**
   * Constructor.
   * @param queryId ID of the executed query.
   */
  DocumentDbCursor(mongocxx::cursor& cursor,
                   std::vector< JdbcColumnMetadata >& columnMetadata,
                   std::vector< std::string >& paths);

  /**
   * Destructor.
   */
  ~DocumentDbCursor();

  /**
   * Move cursor to the next result row.
   *
   * @return False if data update required or no more data.
   */
  bool Increment();

  /**
   * Check if the cursor has data.
   *
   * @return True if the cursor has data.
   */
  bool HasData() const;

  /**
   * Get current row.
   *
   * @return Current row. Returns zero if cursor needs data update or has no
   * more data.
   */
  DocumentDbRow* GetRow();

 private:
  IGNITE_NO_COPY_ASSIGNMENT(DocumentDbCursor);

  /** The resulting cursor to query/aggregate call */
  mongocxx::cursor cursor_;

  /** The iterator to beginning of cursor */
  mongocxx::cursor::iterator iterator_;

  /** The iterator to end of cursor */
  mongocxx::cursor::iterator iteratorEnd_;

  /** The column metadata */
  std::vector< JdbcColumnMetadata > columnMetadata_;

  /** The associated path in the resulting document for each column */
  std::vector< std::string > paths_;

  /** The current row */
  std::unique_ptr< DocumentDbRow > currentRow_;

  // Is this the first row of the iterator?
  bool isFirstRow_ = true;
};
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_DOCUMENTDB_CURSOR
