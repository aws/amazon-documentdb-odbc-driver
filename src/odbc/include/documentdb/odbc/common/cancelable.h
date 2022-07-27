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

/**
 * @file
 * Declares documentdb::odbc::common::Cancelable class.
 */

#ifndef _DOCUMENTDB_ODBC_COMMON_CANCELABLE
#define _DOCUMENTDB_ODBC_COMMON_CANCELABLE

#include <documentdb/odbc/common/common.h>

namespace documentdb {
namespace odbc {
namespace common {
/**
 * Cancelable.
 */
class DOCUMENTDB_IMPORT_EXPORT Cancelable {
 public:
  /**
   * Default constructor.
   */
  Cancelable() {
    // No-op.
  }

  /**
   * Destructor.
   */
  virtual ~Cancelable() {
    // No-op.
  }

  /**
   * Cancels the operation.
   */
  virtual void Cancel() = 0;

 private:
  DOCUMENTDB_NO_COPY_ASSIGNMENT(Cancelable);
};
}  // namespace common
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_COMMON_CANCELABLE
