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
 * Declares documentdb::odbc::commom::Promise class template.
 */

#ifndef _DOCUMENTDB_PROMISE
#define _DOCUMENTDB_PROMISE

#include <documentdb/odbc/common/common.h>
#include <documentdb/odbc/common/shared_state.h>

#include <documentdb/odbc/documentdb_error.h>
#include <documentdb/odbc/future.h>

namespace documentdb {
namespace odbc {
namespace common {
/**
 * Promise class template. Used to set result of the asynchroniously
 * started computation.
 *
 * @tparam T Promised value type.
 */
template < typename T >
class Promise {
 public:
  /** Template value type */
  typedef T ValueType;

  /**
   * Constructor.
   */
  Promise() : state(new SharedState< ValueType >()) {
    // No-op.
  }

  /**
   * Destructor.
   */
  ~Promise() {
    SharedState< ValueType >* state0 = state.Get();

    assert(state0 != 0);

    if (!state0->IsSet())
      state0->SetError(DocumentDbError(
          DocumentDbError::DOCUMENTDB_ERR_FUTURE_STATE,
          "Broken promise. Value will never be set due to internal error."));
  }

  /**
   * Get future for this promise.
   *
   * @return New future instance.
   */
  Future< ValueType > GetFuture() const {
    return Future< ValueType >(state);
  }

  /**
   * Set value.
   *
   * @throw DocumentDbError with DocumentDbError::DOCUMENTDB_ERR_FUTURE_STATE if error or
   * value has been set already.
   * @param val Value to set.
   */
  void SetValue(std::shared_ptr< ValueType > val) {
    SharedState< ValueType >* state0 = state.Get();

    assert(state0 != 0);

    return state0->SetValue(val);
  }

  /**
   * Set error.
   *
   * @throw DocumentDbError with DocumentDbError::DOCUMENTDB_ERR_FUTURE_STATE if error or
   * value has been set already.
   * @param err Error to set.
   */
  void SetError(const DocumentDbError& err) {
    SharedState< ValueType >* state0 = state.Get();

    assert(state0 != 0);

    state0->SetError(err);
  }

  /**
   * Set cancel target.
   */
  void SetCancelTarget(std::shared_ptr< Cancelable >& target) {
    state.Get()->SetCancelTarget(target);
  }

 private:
  DOCUMENTDB_NO_COPY_ASSIGNMENT(Promise);

  /** Shared state. */
  concurrent::SharedPointer< SharedState< ValueType > > state;
};

/**
 * Specialization for void.
 */
template <>
class Promise< void > {
 public:
  /** Template value type */
  typedef void ValueType;

  /**
   * Constructor.
   */
  Promise() : state(new SharedState< ValueType >()) {
    // No-op.
  }

  /**
   * Destructor.
   */
  ~Promise() {
    SharedState< ValueType >* state0 = state.Get();

    assert(state0 != 0);

    if (!state0->IsSet())
      state0->SetError(DocumentDbError(
          DocumentDbError::DOCUMENTDB_ERR_FUTURE_STATE,
          "Broken promise. Value will never be set due to internal error."));
  }

  /**
   * Get future for this promise.
   *
   * @return New future instance.
   */
  Future< ValueType > GetFuture() const {
    return Future< ValueType >(state);
  }

  /**
   * Mark as complete.
   *
   * @throw DocumentDbError with DocumentDbError::DOCUMENTDB_ERR_FUTURE_STATE if error or
   * value has been set already.
   */
  void SetValue() {
    SharedState< ValueType >* state0 = state.Get();

    assert(state0 != 0);

    return state0->SetValue();
  }

  /**
   * Set error.
   *
   * @throw DocumentDbError with DocumentDbError::DOCUMENTDB_ERR_FUTURE_STATE if error or
   * value has been set already.
   * @param err Error to set.
   */
  void SetError(const DocumentDbError& err) {
    SharedState< ValueType >* state0 = state.Get();

    assert(state0 != 0);

    state0->SetError(err);
  }

  /**
   * Set cancel target.
   */
  void SetCancelTarget(std::shared_ptr< Cancelable >& target) {
    state.Get()->SetCancelTarget(target);
  }

 private:
  DOCUMENTDB_NO_COPY_ASSIGNMENT(Promise);

  /** Shared state. */
  concurrent::SharedPointer< SharedState< ValueType > > state;
};
}  // namespace common
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_PROMISE
