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

#ifndef _DOCUMENTDB_ODBC_IMPL_INTEROP_INTEROP_STREAM_POSITION_GUARD
#define _DOCUMENTDB_ODBC_IMPL_INTEROP_INTEROP_STREAM_POSITION_GUARD

#include "documentdb/odbc/impl/interop/interop_memory.h"

namespace documentdb {
namespace odbc {
namespace impl {
namespace interop {
/**
 * Interop stream position guard.
 */
template < typename T >
class DOCUMENTDB_IMPORT_EXPORT InteropStreamPositionGuard {
 public:
  /**
   * Create new position guard and saves current stream position.
   *
   * @param stream Stream which position should be saved.
   */
  InteropStreamPositionGuard(T& stream)
      : stream(&stream), pos(stream.Position()) {
    // No-op
  }

  /**
   * Destructor.
   *
   * Restores stream's position to a saved one on destruction.
   */
  ~InteropStreamPositionGuard() {
    if (stream)
      stream->Position(pos);
  }

  /**
   * Releases guard so it will not restore streams position on destruction.
   *
   * @param val Value.
   */
  void Release() {
    stream = 0;
  }

 private:
  /** Stream. */
  T* stream;

  /** Saved position. */
  int32_t pos;

  DOCUMENTDB_NO_COPY_ASSIGNMENT(InteropStreamPositionGuard);
};
}  // namespace interop
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_IMPL_INTEROP_INTEROP_STREAM_POSITION_GUARD
