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
 * Declares documentdb::odbc::impl::compute::CancelableImpl class.
 */

#ifndef _DOCUMENTDB_ODBC_IMPL_COMPUTE_CANCELABLE_IMPL
#define _DOCUMENTDB_ODBC_IMPL_COMPUTE_CANCELABLE_IMPL

#include <documentdb/odbc/common/common.h>
#include <documentdb/odbc/common/cancelable.h>
#include <documentdb/odbc/impl/interop/interop_target.h>

namespace documentdb {
namespace odbc {
namespace impl {
namespace compute {
/**
 * Compute implementation.
 */
class DOCUMENTDB_IMPORT_EXPORT CancelableImpl : public interop::InteropTarget,
                                            public common::Cancelable {
 public:
  /**
   * Constructor.
   *
   * @param env Environment.
   * @param javaRef Java object reference.
   */
  CancelableImpl(common::concurrent::SharedPointer< IgniteEnvironment > env,
                 jobject javaRef);

  /**
   * Destructor.
   */
  virtual ~CancelableImpl() {
    // No-op.
  }

  /**
   * Cancels the operation.
   */
  virtual void Cancel();

 private:
  DOCUMENTDB_NO_COPY_ASSIGNMENT(CancelableImpl);
};
}  // namespace compute
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_IMPL_COMPUTE_CANCELABLE_IMPL
