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

#include "ignite/odbc/documentdb_cursor.h"
#include "mongocxx/cursor.hpp"

namespace ignite {
namespace odbc {
DocumentDbCursor::DocumentDbCursor(mongocxx::cursor& cursor,
    std::vector< JdbcColumnMetadata >& columnMetadata,
    std::vector< std::string >& paths)
    : cursor_(std::move(cursor)),
      iterator_(cursor_.begin()),
      iteratorEnd_(cursor_.end()),
      columnMetadata_(columnMetadata),
      paths_(paths) {
  // No-op.
}

DocumentDbCursor::~DocumentDbCursor() {
  currentRow_.release();
}

bool DocumentDbCursor::Increment() {
  bool hasData = HasData();
  if (hasData) {
    if (!isFirstRow_) {
      iterator_++;
    } else {
      isFirstRow_ = false;
    }
  }
  hasData = HasData();
  if (hasData) {
    if (currentRow_) {
      (*currentRow_).Update(*iterator_);
    } else {
      currentRow_.reset(new DocumentDbRow(*iterator_, columnMetadata_, paths_));
    }
  } else {
    currentRow_.reset();
  }
  return hasData;
}

bool DocumentDbCursor::HasData() const {
  return iterator_ != iteratorEnd_;
}

DocumentDbRow* DocumentDbCursor::GetRow() {
  return currentRow_.get();
}
}  // namespace odbc
}  // namespace ignite
