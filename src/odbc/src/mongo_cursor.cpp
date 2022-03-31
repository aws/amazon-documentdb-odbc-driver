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

#include "ignite/odbc/mongo_cursor.h"
#include "mongocxx/cursor.hpp"

namespace ignite {
namespace odbc {
MongoCursor::MongoCursor(mongocxx::cursor& cursor,
    std::vector< JdbcColumnMetadata >& columnMetadata,
    std::vector< std::string >& paths)
    : cursor_(std::move(cursor)),
      iterator_(cursor_.begin()),
      iteratorEnd_(cursor_.end()),
      columnMetadata_(columnMetadata),
      paths_(paths) {
  // No-op.
}

MongoCursor::~MongoCursor() {
  currentRow_.release();
}

bool MongoCursor::Increment() {
  bool hasData = HasData();
  if (hasData) {
    currentRow_.reset(new MongoRow(iterator_, iteratorEnd_, columnMetadata_, paths_));
    iterator_++;
  } else {
    currentRow_.reset();
  }
  return hasData;
}

bool MongoCursor::HasData() const {
  return iterator_ != iteratorEnd_;
}

MongoRow* MongoCursor::GetRow() {
  return currentRow_.get();
}
}  // namespace odbc
}  // namespace ignite
