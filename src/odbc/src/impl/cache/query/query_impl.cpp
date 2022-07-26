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

#include "documentdb/odbc/impl/cache/query/query_impl.h"
#include "documentdb/odbc/impl/cache/query/query_fields_row_impl.h"

using namespace documentdb::odbc::common::concurrent;
using namespace documentdb::odbc::jni::java;
using namespace documentdb::odbc::impl::interop;
using namespace documentdb::odbc::impl::binary;

namespace documentdb {
namespace odbc {
namespace impl {
namespace cache {
namespace query {
/** Operation: get all entries. */
const int32_t OP_GET_ALL = 1;

/** Operation: get multiple entries. */
const int32_t OP_GET_BATCH = 2;

/** Operation: start iterator. */
const int32_t OP_ITERATOR = 4;

/** Operation: close iterator. */
const int32_t OP_ITERATOR_CLOSE = 5;

/** Operation: close iterator. */
const int32_t OP_ITERATOR_HAS_NEXT = 6;

QueryCursorImpl::QueryCursorImpl(SharedPointer< IgniteEnvironment > env,
                                 jobject javaRef)
    : env(env),
      javaRef(javaRef),
      batch(0),
      endReached(false),
      iterCalled(false),
      getAllCalled(false) {
  // No-op.
}

QueryCursorImpl::~QueryCursorImpl() {
  // 1. Releasing memory.
  delete batch;

  // 2. Close the cursor.
  JniErrorInfo err;
  env.Get()->Context()->TargetInLongOutLong(javaRef, OP_ITERATOR_CLOSE, 0,
                                            &err);

  // 3. Release Java reference.
  JniContext::Release(javaRef);
}

bool QueryCursorImpl::HasNext(DocumentDbError& err) {
  // Check whether GetAll() was called earlier.
  if (getAllCalled) {
    err =
        DocumentDbError(DocumentDbError::IGNITE_ERR_GENERIC,
                    "Cannot use HasNext() method because GetAll() was called.");

    return false;
  }

  // Create iterator in Java if needed.
  if (!CreateIteratorIfNeeded(err))
    return false;

  // Get next results batch if the end in the current batch
  // has been reached.
  if (!GetNextBatchIfNeeded(err))
    return false;

  return !endReached;
}

void QueryCursorImpl::GetNext(OutputOperation& op, DocumentDbError& err) {
  // Check whether GetAll() was called earlier.
  if (getAllCalled) {
    err =
        DocumentDbError(DocumentDbError::IGNITE_ERR_GENERIC,
                    "Cannot use GetNext() method because GetAll() was called.");

    return;
  }

  // Create iterator in Java if needed.
  if (!CreateIteratorIfNeeded(err))
    return;

  // Get next results batch if the end in the current batch
  // has been reached.
  if (!GetNextBatchIfNeeded(err))
    return;

  if (endReached) {
    // Ensure we do not overwrite possible previous error.
    if (err.GetCode() == DocumentDbError::IGNITE_SUCCESS)
      err = DocumentDbError(DocumentDbError::IGNITE_ERR_GENERIC,
                        "No more elements available.");

    return;
  }

  batch->GetNext(op);
}

QueryFieldsRowImpl* QueryCursorImpl::GetNextRow(DocumentDbError& err) {
  // Create iterator in Java if needed.
  if (!CreateIteratorIfNeeded(err))
    return 0;

  // Get next results batch if the end in the current batch
  // has been reached.
  if (!GetNextBatchIfNeeded(err))
    return 0;

  if (endReached) {
    // Ensure we do not overwrite possible previous error.
    if (err.GetCode() == DocumentDbError::IGNITE_SUCCESS)
      err = DocumentDbError(DocumentDbError::IGNITE_ERR_GENERIC,
                        "No more elements available.");

    return 0;
  }

  return batch->GetNextRow();
}

void QueryCursorImpl::GetAll(OutputOperation& op, DocumentDbError& err) {
  // Check whether any of iterator methods were called.
  if (iterCalled) {
    err = DocumentDbError(
        DocumentDbError::IGNITE_ERR_GENERIC,
        "Cannot use GetAll() method because an iteration method was called.");

    return;
  }

  // Check whether GetAll was called before.
  if (getAllCalled) {
    err =
        DocumentDbError(DocumentDbError::IGNITE_ERR_GENERIC,
                    "Cannot use GetNext() method because GetAll() was called.");

    return;
  }

  // Get data.
  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > inMem = env.Get()->AllocateMemory();

  env.Get()->Context()->TargetOutStream(javaRef, OP_GET_ALL,
                                        inMem.Get()->PointerLong(), &jniErr);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);

  if (jniErr.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    getAllCalled = true;

    InteropInputStream in(inMem.Get());

    BinaryReaderImpl reader(&in);

    op.ProcessOutput(reader);
  }
}

void QueryCursorImpl::GetAll(OutputOperation& op) {
  // Check whether any of iterator methods were called.
  if (iterCalled) {
    throw DocumentDbError(
        DocumentDbError::IGNITE_ERR_GENERIC,
        "Cannot use GetAll() method because an iteration method was called.");
  }

  // Check whether GetAll was called before.
  if (getAllCalled) {
    throw DocumentDbError(
        DocumentDbError::IGNITE_ERR_GENERIC,
        "Cannot use GetNext() method because GetAll() was called.");
  }

  // Get data.
  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > inMem = env.Get()->AllocateMemory();

  env.Get()->Context()->TargetOutStream(javaRef, OP_GET_ALL,
                                        inMem.Get()->PointerLong(), &jniErr);

  DocumentDbError err;
  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);

  DocumentDbError::ThrowIfNeeded(err);

  getAllCalled = true;

  InteropInputStream in(inMem.Get());

  BinaryReaderImpl reader(&in);

  op.ProcessOutput(reader);
}

bool QueryCursorImpl::CreateIteratorIfNeeded(DocumentDbError& err) {
  if (iterCalled)
    return true;

  JniErrorInfo jniErr;

  env.Get()->Context()->TargetInLongOutLong(javaRef, OP_ITERATOR, 0, &jniErr);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);

  if (jniErr.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS)
    iterCalled = true;

  return iterCalled;
}

bool QueryCursorImpl::GetNextBatchIfNeeded(DocumentDbError& err) {
  assert(iterCalled);

  if (endReached || (batch && batch->Left() > 0))
    return true;

  endReached = !IteratorHasNext(err);

  if (endReached)
    return true;

  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > inMem = env.Get()->AllocateMemory();

  env.Get()->Context()->TargetOutStream(javaRef, OP_GET_BATCH,
                                        inMem.Get()->PointerLong(), &jniErr);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);

  if (jniErr.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS)
    return false;

  delete batch;

  // Needed for exception safety.
  batch = 0;

  batch = new QueryBatch(*env.Get(), inMem);

  endReached = batch->IsEmpty();

  return true;
}

bool QueryCursorImpl::IteratorHasNext(DocumentDbError& err) {
  JniErrorInfo jniErr;

  bool res = env.Get()->Context()->TargetInLongOutLong(
                 javaRef, OP_ITERATOR_HAS_NEXT, 0, &jniErr)
             == 1;

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);

  if (jniErr.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS)
    return res;

  return false;
}
}  // namespace query
}  // namespace cache
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb
