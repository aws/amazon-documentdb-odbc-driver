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

#ifndef _DOCUMENTDB_ODBC_CURSOR
#define _DOCUMENTDB_ODBC_CURSOR

#include <stdint.h>

#include <map>
#include <memory>

#include "documentdb/odbc/common_types.h"
#include "documentdb/odbc/result_page.h"
#include "documentdb/odbc/row.h"

namespace documentdb {
namespace odbc {
/**
 * Query result cursor.
 */
class Cursor {
 public:
  /**
   * Constructor.
   * @param queryId ID of the executed query.
   */
  Cursor(int64_t queryId);

  /**
   * Destructor.
   */
  ~Cursor();

  /**
   * Move cursor to the next result row.
   *
   * @return False if data update required or no more data.
   */
  bool Increment();

  /**
   * Check if the cursor needs data update.
   *
   * @return True if the cursor needs data update.
   */
  bool NeedDataUpdate() const;

  /**
   * Check if the cursor has data.
   *
   * @return True if the cursor has data.
   */
  bool HasData() const;

  /**
   * Check whether cursor closed remotely.
   *
   * @return true, if the cursor closed remotely.
   */
  bool IsClosedRemotely() const;

  /**
   * Get query ID.
   *
   * @return Query ID.
   */
  int64_t GetQueryId() const {
    return queryId;
  }

  /**
   * Update current cursor page data.
   *
   * @param newPage New result page.
   */
  void UpdateData(std::shared_ptr< ResultPage >& newPage);

  /**
   * Get current row.
   *
   * @return Current row. Returns zero if cursor needs data update or has no
   * more data.
   */
  Row* GetRow();

 private:
  DOCUMENTDB_NO_COPY_ASSIGNMENT(Cursor);

  /** Cursor id. */
  int64_t queryId;

  /** Current page. */
  std::shared_ptr< ResultPage > currentPage;

  /** Row position in current page. */
  int32_t currentPagePos;

  /** Current row. */
  std::unique_ptr< Row > currentRow;
};
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_CURSOR
