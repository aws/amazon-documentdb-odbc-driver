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
 * Declares documentdb::odbc::IgniteBinding class.
 */

#ifndef _DOCUMENTDB_ODBC_IGNITE_BINDING
#define _DOCUMENTDB_ODBC_IGNITE_BINDING

#include <documentdb/odbc/common/common.h>
#include <documentdb/odbc/common/concurrent.h>

#include <documentdb/odbc/impl/ignite_binding_impl.h>
#include <documentdb/odbc/impl/bindings.h>

namespace documentdb {
namespace odbc {
/**
 * %Ignite Binding.
 * Used to register callable classes.
 */
class DOCUMENTDB_IMPORT_EXPORT IgniteBinding {
 public:
  /**
   * Default constructor.
   */
  IgniteBinding() : impl() {
    // No-op.
  }

  /**
   * Constructor.
   *
   * @param impl Implementation.
   */
  IgniteBinding(
      common::concurrent::SharedPointer< impl::IgniteBindingImpl > impl)
      : impl(impl) {
    // No-op.
  }

  /**
   * Register type as Cache Entry Processor.
   *
   * Registred type should be a child of
   * documentdb::odbc::cache::CacheEntryProcessor class.
   */
  template < typename P >
  void RegisterCacheEntryProcessor() {
    DocumentDbError err;

    RegisterCacheEntryProcessor< P >(err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Register Type as Cache Entry Processor.
   *
   * Registred type should be a child of
   * documentdb::odbc::cache::CacheEntryProcessor class.
   *
   * @param err Error.
   */
  template < typename P >
  void RegisterCacheEntryProcessor(DocumentDbError &err) {
    impl::IgniteBindingImpl *im = impl.Get();

    if (im) {
      im->RegisterCallback(
          impl::IgniteBindingImpl::CallbackType::CACHE_ENTRY_PROCESSOR_APPLY,
          binary::BinaryType< P >::GetTypeId(),
          impl::binding::ListenerApply<
              P, typename P::KeyType, typename P::ValueType,
              typename P::ReturnType, typename P::ArgumentType >,
          err);
    } else {
      err = DocumentDbError(DocumentDbError::DOCUMENTDB_ERR_GENERIC,
                        "Instance is not usable (did you check for error?).");
    }
  }

  /**
   * Register type as Cache Entry Event Filter.
   *
   * Registred type should be a child of
   * documentdb::odbc::cache::event::CacheEntryEventFilter class.
   */
  template < typename F >
  void RegisterCacheEntryEventFilter() {
    impl::IgniteBindingImpl *im = impl.Get();

    int32_t typeId = binary::BinaryType< F >::GetTypeId();

    if (im) {
      im->RegisterCallback(
          impl::IgniteBindingImpl::CallbackType::CACHE_ENTRY_FILTER_CREATE,
          typeId, impl::binding::FilterCreate< F >);
    } else {
      throw DocumentDbError(DocumentDbError::DOCUMENTDB_ERR_GENERIC,
                        "Instance is not usable (did you check for error?).");
    }
  }

  /**
   * Register type as Compute function.
   *
   * Registred type should be a child of documentdb::odbc::compute::ComputeFunc
   * class.
   */
  template < typename F >
  void RegisterComputeFunc() {
    impl::IgniteBindingImpl *im = impl.Get();

    int32_t typeId = binary::BinaryType< F >::GetTypeId();

    if (im) {
      im->RegisterCallback(
          impl::IgniteBindingImpl::CallbackType::COMPUTE_JOB_CREATE, typeId,
          impl::binding::ComputeJobCreate< F, typename F::ReturnType >);
    } else {
      throw DocumentDbError(DocumentDbError::DOCUMENTDB_ERR_GENERIC,
                        "Instance is not usable (did you check for error?).");
    }
  }

  /**
   * Check if the instance is valid.
   *
   * Invalid instance can be returned if some of the previous operations
   * have resulted in a failure. For example invalid instance can be
   * returned by not-throwing version of method in case of error. Invalid
   * instances also often can be created using default constructor.
   *
   * @return True if the instance is valid and can be used.
   */
  bool IsValid() const {
    return impl.IsValid();
  }

 private:
  /** Registered cache entry processors. */
  common::concurrent::SharedPointer< impl::IgniteBindingImpl > impl;
};
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_IGNITE_BINDING
