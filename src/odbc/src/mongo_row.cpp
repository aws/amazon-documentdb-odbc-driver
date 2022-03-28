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
#include "ignite/odbc/mongo_column.h"
#include "ignite/odbc/mongo_row.h"
#include "ignite/odbc/utility.h"
#include "mongocxx/cursor.hpp"

using namespace ignite::odbc::impl::interop;
using ignite::odbc::jni::JdbcColumnMetadata;

namespace ignite {
namespace odbc {
// ASSUMPTION: iterator is not at the end.
MongoRow::MongoRow(mongocxx::cursor::iterator& iterator,
                   mongocxx::cursor::iterator& iteratorEnd,
                   std::vector< JdbcColumnMetadata >& columnMetadata,
                   std::vector< std::string >& paths)
    : pos(0),
      size(columnMetadata.size()),
      columns(),
      _iterator(iterator),
      _iteratorEnd(iteratorEnd),
      _document(*iterator),
      _columnMetadata(columnMetadata),
      _paths(paths) {
}

MongoRow::~MongoRow() {
  // No-op.
}

app::ConversionResult::Type MongoRow::ReadColumnToBuffer(
    uint16_t columnIdx, app::ApplicationDataBuffer& dataBuf) {
  if (columnIdx > GetSize() || columnIdx < 1)
    return app::ConversionResult::AI_FAILURE;

  if (!EnsureColumnDiscovered(columnIdx))
    return app::ConversionResult::AI_FAILURE;

  MongoColumn& column = GetColumn(columnIdx);

  return column.ReadToBuffer(dataBuf);
}

bool MongoRow::MoveToNext() {
  return ++_iterator  != _iteratorEnd;
}

bool MongoRow::EnsureColumnDiscovered(int16_t columnIdx) {
  if (columns.size() == size)
    return true;

  int64_t index = columns.size();
  while (columns.size() < columnIdx) {
    MongoColumn newColumn(_document, _columnMetadata[index], _paths[index]);

    if (!newColumn.IsValid())
      return false;

    columns.push_back(newColumn);
    index++;
  }

  return true;
}

void MongoRow::Reinit() {
}
}  // namespace odbc
}  // namespace ignite
