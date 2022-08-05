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

#ifndef _DOCUMENTDB_ODBC_IMPL_IGNITE_BINDING_IMPL
#define _DOCUMENTDB_ODBC_IMPL_IGNITE_BINDING_IMPL

#include <stdint.h>
#include <map>

#include <documentdb/odbc/common/common.h>

#include <documentdb/odbc/impl/binary/binary_reader_impl.h>
#include <documentdb/odbc/impl/binary/binary_writer_impl.h>

namespace documentdb {
namespace odbc {
namespace impl {
/* Forward declaration. */
class IgniteEnvironment;

/**
 * Ignite binding implementation.
 *
 * Used to register and invoke callbacks.
 */
class IgniteBindingImpl {
  typedef int64_t(Callback)(binary::BinaryReaderImpl&,
                            binary::BinaryWriterImpl&, IgniteEnvironment&);

 public:
  struct CallbackType {
    enum Type {
      CACHE_ENTRY_PROCESSOR_APPLY = 1,

      CACHE_ENTRY_FILTER_CREATE = 2,

      CACHE_ENTRY_FILTER_APPLY = 3,

      COMPUTE_JOB_CREATE = 4
    };
  };

  /**
   * Constructor.
   *
   * @param env Environment.
   */
  IgniteBindingImpl(IgniteEnvironment& env);

  /**
   * Invoke callback using provided ID.
   *
   * Deserializes data and callback itself, invokes callback and
   * serializes processing result using providede reader and writer.
   *
   * @param type Callback Type.
   * @param id Callback ID.
   * @param reader Reader.
   * @param writer Writer.
   * @param found Output param. True if callback was found and false otherwise.
   * @return Callback return value.
   */
  DOCUMENTDB_IMPORT_EXPORT int64_t InvokeCallback(bool& found, int32_t type,
                                              int32_t id,
                                              binary::BinaryReaderImpl& reader,
                                              binary::BinaryWriterImpl& writer);

  /**
   * Register cache entry processor and associate it with provided ID.
   *
   * @throw DocumentDbError another processor is already associated with
   *     the given ID.
   *
   * @param type Callback type.
   * @param id Callback identifier.
   * @param callback Callback.
   * @param err Error.
   */
  DOCUMENTDB_IMPORT_EXPORT void RegisterCallback(int32_t type, int32_t id,
                                             Callback* callback,
                                             DocumentDbError& err);

  /**
   * Register cache entry processor and associate it with provided ID.
   *
   * @throw DocumentDbError another processor is already associated with
   *     the given ID.
   *
   * @param type Callback type.
   * @param id Callback identifier.
   * @param callback Callback.
   */
  DOCUMENTDB_IMPORT_EXPORT void RegisterCallback(int32_t type, int32_t id,
                                             Callback* callback);

 private:
  DOCUMENTDB_NO_COPY_ASSIGNMENT(IgniteBindingImpl);

  /**
   * Make key out of callback's type and ID.
   *
   * @param type Callback Type.
   * @param id Callback ID.
   * @return Key for callback.
   */
  int64_t makeKey(int32_t type, int32_t id) {
    return ((static_cast< int64_t >(type) & 0xFFFFFFFF) << 32)
           | ((static_cast< int64_t >(id) & 0xFFFFFFFF));
  }

  /** Ignite environment. */
  IgniteEnvironment& env;

  /** Registered callbacks. */
  std::map< int64_t, Callback* > callbacks;

  /** Callback lock. */
  common::concurrent::CriticalSection lock;
};
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_IMPL_IGNITE_BINDING_IMPL
