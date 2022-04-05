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

#include "ignite/odbc/jni/jdbc_column_metadata.h"
#include "ignite/odbc/documentdb_row.h"
#include "ignite/odbc/utility.h"
#include "mongocxx/cursor.hpp"

using namespace ignite::odbc::impl::interop;
using ignite::odbc::jni::JdbcColumnMetadata;

namespace ignite {
namespace odbc {
// ASSUMPTION: iterator is not at the end.
DocumentDbRow::DocumentDbRow(bsoncxx::document::view const& document,
                   std::vector< JdbcColumnMetadata >& columnMetadata,
                   std::vector< std::string >& paths)
    : pos(0),
      size(columnMetadata.size()),
      columns_(),
      document_(document),
      columnMetadata_(columnMetadata),
      paths_(paths) {
}

void DocumentDbRow::Update(bsoncxx::document::view const& document) {
  document_ = document;
  for (DocumentDbColumn& column : columns_) {
    column.Update(document);
  }
}

app::ConversionResult::Type DocumentDbRow::ReadColumnToBuffer(
    uint32_t columnIdx, app::ApplicationDataBuffer& dataBuf) {
  if (columnIdx > GetSize() || columnIdx < 1)
    return app::ConversionResult::AI_FAILURE;

  if (!EnsureColumnDiscovered(columnIdx))
    return app::ConversionResult::AI_FAILURE;

  DocumentDbColumn const& column = GetColumn(columnIdx);

  return column.ReadToBuffer(dataBuf);
}

bool DocumentDbRow::EnsureColumnDiscovered(uint32_t columnIdx) {
  if (columns_.size() == size)
    return true;

  int64_t index = columns_.size();
  while (columns_.size() < columnIdx) {
    DocumentDbColumn newColumn(document_, columnMetadata_[index], paths_[index]);

    columns_.push_back(newColumn);
    index++;
  }

  return true;
}
}  // namespace odbc
}  // namespace ignite
