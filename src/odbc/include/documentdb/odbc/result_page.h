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

#ifndef _DOCUMENTDB_ODBC_RESULT_PAGE
#define _DOCUMENTDB_ODBC_RESULT_PAGE

#include <documentdb/odbc/impl/binary/binary_reader_impl.h>
#include <stdint.h>

#include "documentdb/odbc/app/application_data_buffer.h"
#include "documentdb/odbc/common_types.h"

namespace documentdb {
namespace odbc {
/**
 * Query result page.
 */
class ResultPage {
  enum { DEFAULT_ALLOCATED_MEMORY = 1024 };

 public:
  /**
   * Constructor.
   */
  ResultPage();

  /**
   * Destructor.
   */
  ~ResultPage();

  /**
   * Read result page using provided reader.
   * @param reader Reader.
   */
  void Read(documentdb::odbc::impl::binary::BinaryReaderImpl& reader);

  /**
   * Get page size.
   * @return Page size.
   */
  int32_t GetSize() const {
    return size;
  }

  /**
   * Check if the page is last.
   * @return True if the page is last.
   */
  bool IsLast() const {
    return last;
  }

  /**
   * Get page data.
   * @return Page data.
   */
  documentdb::odbc::impl::interop::InteropUnpooledMemory& GetData() {
    return data;
  }

 private:
  DOCUMENTDB_NO_COPY_ASSIGNMENT(ResultPage);

  /** Last page flag. */
  bool last;

  /** Page size in rows. */
  int32_t size;

  /** Memory that contains current row page data. */
  documentdb::odbc::impl::interop::InteropUnpooledMemory data;
};
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_RESULT_PAGE
