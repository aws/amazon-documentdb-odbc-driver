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

#include <documentdb/odbc/common/utils.h>

#include <documentdb/odbc/impl/cache/cache_impl.h>
#include <documentdb/odbc/impl/binary/binary_type_updater_impl.h>
#include <documentdb/odbc/impl/cache/query/continuous/continuous_query_handle_impl.h>

#include <documentdb/odbc/cache/query/continuous/continuous_query_handle.h>

using namespace documentdb::odbc::common::concurrent;
using namespace documentdb::odbc::jni::java;
using namespace documentdb::odbc::common;
using namespace documentdb::odbc::cache;
using namespace documentdb::odbc::cache::query;
using namespace documentdb::odbc::cache::query::continuous;
using namespace documentdb::odbc::impl;
using namespace documentdb::odbc::impl::binary;
using namespace documentdb::odbc::impl::cache::query;
using namespace documentdb::odbc::impl::cache::query::continuous;
using namespace documentdb::odbc::impl::interop;
using namespace documentdb::odbc::binary;

struct Operation {
  enum Type {
    /** Operation: Clear. */
    CLEAR = 1,

    /** Operation: ClearAll. */
    CLEAR_ALL = 2,

    /** Operation: ContainsKey. */
    CONTAINS_KEY = 3,

    /** Operation: ContainsKeys. */
    CONTAINS_KEYS = 4,

    /** Operation: Get. */
    GET = 5,

    /** Operation: GetAll. */
    GET_ALL = 6,

    /** Operation: GetAndPut. */
    GET_AND_PUT = 7,

    /** Operation: GetAndPutIfAbsent. */
    GET_AND_PUT_IF_ABSENT = 8,

    /** Operation: GetAndRemove. */
    GET_AND_REMOVE = 9,

    /** Operation: GetAndReplace. */
    GET_AND_REPLACE = 10,

    /** Operation: Invoke. */
    INVOKE = 12,

    /** Operation: LoadCache */
    LOAD_CACHE = 15,

    /** Operation: LocalEvict. */
    LOCAL_EVICT = 16,

    /** Operation: LocalLoadCache */
    LOC_LOAD_CACHE = 17,

    /** Operation: LocalClear. */
    LOCAL_CLEAR = 20,

    /** Operation: LocalClearAll. */
    LOCAL_CLEAR_ALL = 21,

    /** Operation: LocalPeek. */
    LOCAL_PEEK = 25,

    /** Operation: Put. */
    PUT = 26,

    /** Operation: PutAll. */
    PUT_ALL = 27,

    /** Operation: PutIfAbsent. */
    PUT_IF_ABSENT = 28,

    /** Operation: CONTINUOUS query. */
    QRY_CONTINUOUS = 29,

    /** Operation: SCAN query. */
    QRY_SCAN = 30,

    /** Operation: SQL query. */
    QRY_SQL = 31,

    /** Operation: SQL fields query. */
    QRY_SQL_FIELDS = 32,

    /** Operation: TEXT query. */
    QRY_TEXT = 33,

    /** Operation: RemoveAll. */
    REMOVE_ALL = 34,

    /** Operation: Remove(K, V). */
    REMOVE_2 = 35,

    /** Operation: Remove(K). */
    REMOVE_1 = 36,

    /** Operation: Replace(K, V). */
    REPLACE_2 = 37,

    /** Operation: Replace(K, V, V). */
    REPLACE_3 = 38,

    /** Operation: Clear(). */
    CLEAR_CACHE = 41,

    /** Operation: RemoveAll(). */
    REMOVE_ALL2 = 43,

    /** Operation: Size(peekModes). */
    SIZE = 48,

    /** Operation: SizeLoc(peekModes). */
    SIZE_LOC = 56,

    /** Operation: Invoke. */
    INVOKE_JAVA = 98,
  };
};

namespace documentdb {
namespace odbc {
namespace impl {
namespace cache {

CacheImpl::CacheImpl(char* name, SharedPointer< IgniteEnvironment > env,
                     jobject javaRef)
    : InteropTarget(env, javaRef), name(name) {
  // No-op.
}

CacheImpl::~CacheImpl() {
  ReleaseChars(name);

  JniContext::Release(GetTarget());
}

const char* CacheImpl::GetName() const {
  return name;
}

bool CacheImpl::ContainsKey(InputOperation& inOp, DocumentDbError& err) {
  return OutOp(Operation::CONTAINS_KEY, inOp, err);
}

bool CacheImpl::ContainsKeys(InputOperation& inOp, DocumentDbError& err) {
  return OutOp(Operation::CONTAINS_KEYS, inOp, err);
}

void CacheImpl::LocalPeek(InputOperation& inOp, OutputOperation& outOp,
                          DocumentDbError& err) {
  OutInOpX(Operation::LOCAL_PEEK, inOp, outOp, err);
}

void CacheImpl::Get(InputOperation& inOp, OutputOperation& outOp,
                    DocumentDbError& err) {
  OutInOpX(Operation::GET, inOp, outOp, err);
}

void CacheImpl::GetAll(InputOperation& inOp, OutputOperation& outOp,
                       DocumentDbError& err) {
  OutInOpX(Operation::GET_ALL, inOp, outOp, err);
}

void CacheImpl::Put(InputOperation& inOp, DocumentDbError& err) {
  OutOp(Operation::PUT, inOp, err);
}

void CacheImpl::PutAll(documentdb::odbc::impl::InputOperation& inOp,
                       DocumentDbError& err) {
  OutOp(Operation::PUT_ALL, inOp, err);
}

void CacheImpl::GetAndPut(InputOperation& inOp, OutputOperation& outOp,
                          DocumentDbError& err) {
  OutInOpX(Operation::GET_AND_PUT, inOp, outOp, err);
}

void CacheImpl::GetAndReplace(InputOperation& inOp, OutputOperation& outOp,
                              DocumentDbError& err) {
  OutInOpX(Operation::GET_AND_REPLACE, inOp, outOp, err);
}

void CacheImpl::GetAndRemove(InputOperation& inOp, OutputOperation& outOp,
                             DocumentDbError& err) {
  OutInOpX(Operation::GET_AND_REMOVE, inOp, outOp, err);
}

bool CacheImpl::PutIfAbsent(InputOperation& inOp, DocumentDbError& err) {
  return OutOp(Operation::PUT_IF_ABSENT, inOp, err);
}

void CacheImpl::GetAndPutIfAbsent(InputOperation& inOp, OutputOperation& outOp,
                                  DocumentDbError& err) {
  OutInOpX(Operation::GET_AND_PUT_IF_ABSENT, inOp, outOp, err);
}

bool CacheImpl::Replace(InputOperation& inOp, DocumentDbError& err) {
  return OutOp(Operation::REPLACE_2, inOp, err);
}

bool CacheImpl::ReplaceIfEqual(InputOperation& inOp, DocumentDbError& err) {
  return OutOp(Operation::REPLACE_3, inOp, err);
}

void CacheImpl::LocalEvict(InputOperation& inOp, DocumentDbError& err) {
  OutOp(Operation::LOCAL_EVICT, inOp, err);
}

void CacheImpl::Clear(DocumentDbError& err) {
  JniErrorInfo jniErr;

  OutOp(Operation::CLEAR_CACHE, err);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);
}

void CacheImpl::Clear(InputOperation& inOp, DocumentDbError& err) {
  OutOp(Operation::CLEAR, inOp, err);
}

void CacheImpl::ClearAll(InputOperation& inOp, DocumentDbError& err) {
  OutOp(Operation::CLEAR_ALL, inOp, err);
}

void CacheImpl::LocalClear(InputOperation& inOp, DocumentDbError& err) {
  OutOp(Operation::LOCAL_CLEAR, inOp, err);
}

void CacheImpl::LocalClearAll(InputOperation& inOp, DocumentDbError& err) {
  OutOp(Operation::LOCAL_CLEAR_ALL, inOp, err);
}

bool CacheImpl::Remove(InputOperation& inOp, DocumentDbError& err) {
  return OutOp(Operation::REMOVE_1, inOp, err);
}

bool CacheImpl::RemoveIfEqual(InputOperation& inOp, DocumentDbError& err) {
  return OutOp(Operation::REMOVE_2, inOp, err);
}

void CacheImpl::RemoveAll(InputOperation& inOp, DocumentDbError& err) {
  OutOp(Operation::REMOVE_ALL, inOp, err);
}

void CacheImpl::RemoveAll(DocumentDbError& err) {
  JniErrorInfo jniErr;

  OutOp(Operation::REMOVE_ALL2, err);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);
}

int32_t CacheImpl::Size(int32_t peekModes, bool local, DocumentDbError& err) {
  int32_t op = local ? Operation::SIZE_LOC : Operation::SIZE;

  return static_cast< int32_t >(OutInOpLong(op, peekModes, err));
}

QueryCursorImpl* CacheImpl::QuerySql(const SqlQuery& qry, DocumentDbError& err) {
  return QueryInternal(qry, Operation::QRY_SQL, err);
}

QueryCursorImpl* CacheImpl::QueryText(const TextQuery& qry, DocumentDbError& err) {
  return QueryInternal(qry, Operation::QRY_TEXT, err);
}

QueryCursorImpl* CacheImpl::QueryScan(const ScanQuery& qry, DocumentDbError& err) {
  return QueryInternal(qry, Operation::QRY_SCAN, err);
}

void CacheImpl::Invoke(InputOperation& inOp, OutputOperation& outOp,
                       DocumentDbError& err) {
  OutInOpX(Operation::INVOKE, inOp, outOp, err);
}

void CacheImpl::InvokeJava(InputOperation& inOp, OutputOperation& outOp,
                           DocumentDbError& err) {
  OutInOpX(Operation::INVOKE_JAVA, inOp, outOp, err);
}

QueryCursorImpl* CacheImpl::QuerySqlFields(const SqlFieldsQuery& qry,
                                           DocumentDbError& err) {
  return QueryInternal(qry, Operation::QRY_SQL_FIELDS, err);
}

ContinuousQueryHandleImpl* CacheImpl::QueryContinuous(
    const SharedPointer< ContinuousQueryImplBase > qry,
    const SqlQuery& initialQry, DocumentDbError& err) {
  return QueryContinuous(qry, initialQry, Operation::QRY_SQL,
                         Operation::QRY_CONTINUOUS, err);
}

ContinuousQueryHandleImpl* CacheImpl::QueryContinuous(
    const SharedPointer< ContinuousQueryImplBase > qry,
    const TextQuery& initialQry, DocumentDbError& err) {
  return QueryContinuous(qry, initialQry, Operation::QRY_TEXT,
                         Operation::QRY_CONTINUOUS, err);
}

ContinuousQueryHandleImpl* CacheImpl::QueryContinuous(
    const SharedPointer< ContinuousQueryImplBase > qry,
    const ScanQuery& initialQry, DocumentDbError& err) {
  return QueryContinuous(qry, initialQry, Operation::QRY_SCAN,
                         Operation::QRY_CONTINUOUS, err);
}

void CacheImpl::LoadCache(DocumentDbError& err) {
  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > mem = GetEnvironment().AllocateMemory();
  InteropOutputStream out(mem.Get());
  BinaryWriterImpl writer(&out, GetEnvironment().GetTypeManager());

  // Predicate. Always null for now.
  writer.WriteNull();

  // Arguments. No arguments supported for now.
  writer.WriteInt32(0);

  out.Synchronize();

  InStreamOutLong(Operation::LOAD_CACHE, *mem.Get(), err);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);
}

void CacheImpl::LocalLoadCache(DocumentDbError& err) {
  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > mem = GetEnvironment().AllocateMemory();
  InteropOutputStream out(mem.Get());
  BinaryWriterImpl writer(&out, GetEnvironment().GetTypeManager());

  // Predicate. Always null for now.
  writer.WriteNull();

  // Arguments. No arguments supported for now.
  writer.WriteInt32(0);

  out.Synchronize();

  InStreamOutLong(Operation::LOC_LOAD_CACHE, *mem.Get(), err);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);
}

struct Dummy {
  void Write(BinaryRawWriter&) const {
    // No-op.
  }
};

ContinuousQueryHandleImpl* CacheImpl::QueryContinuous(
    const SharedPointer< ContinuousQueryImplBase > qry, DocumentDbError& err) {
  Dummy dummy;
  return QueryContinuous(qry, dummy, -1, Operation::QRY_CONTINUOUS, err);
}

template < typename T >
QueryCursorImpl* CacheImpl::QueryInternal(const T& qry, int32_t typ,
                                          DocumentDbError& err) {
  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > mem = GetEnvironment().AllocateMemory();
  InteropMemory* mem0 = mem.Get();
  InteropOutputStream out(mem0);
  BinaryWriterImpl writer(&out, GetEnvironment().GetTypeManager());
  BinaryRawWriter rawWriter(&writer);

  qry.Write(rawWriter);

  out.Synchronize();

  jobject qryJavaRef = GetEnvironment().Context()->CacheOutOpQueryCursor(
      GetTarget(), typ, mem.Get()->PointerLong(), &jniErr);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);

  if (jniErr.code == JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS)
    return new QueryCursorImpl(GetEnvironmentPointer(), qryJavaRef);

  return 0;
}

template < typename T >
ContinuousQueryHandleImpl* CacheImpl::QueryContinuous(
    const SharedPointer< ContinuousQueryImplBase > qry, const T& initialQry,
    int32_t typ, int32_t cmd, DocumentDbError& err) {
  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > mem = GetEnvironment().AllocateMemory();
  InteropMemory* mem0 = mem.Get();
  InteropOutputStream out(mem0);
  BinaryWriterImpl writer(&out, GetEnvironment().GetTypeManager());
  BinaryRawWriter rawWriter(&writer);

  const ContinuousQueryImplBase& qry0 = *qry.Get();

  int64_t handle = GetEnvironment().GetHandleRegistry().Allocate(qry);

  rawWriter.WriteInt64(handle);
  rawWriter.WriteBool(qry0.GetLocal());
  rawWriter.WriteBool(false);  // IncludeExpired

  event::CacheEntryEventFilterHolderBase& filterOp = qry0.GetFilterHolder();

  filterOp.Write(writer);

  rawWriter.WriteInt32(qry0.GetBufferSize());
  rawWriter.WriteInt64(qry0.GetTimeInterval());

  // Autounsubscribe is a filter feature.
  rawWriter.WriteBool(false);

  // Writing initial query. When there is not initial query writing -1.
  rawWriter.WriteInt32(typ);
  if (typ != -1)
    initialQry.Write(rawWriter);

  out.Synchronize();

  jobject qryJavaRef = GetEnvironment().Context()->CacheOutOpContinuousQuery(
      GetTarget(), cmd, mem.Get()->PointerLong(), &jniErr);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);

  if (jniErr.code == JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS)
    return new ContinuousQueryHandleImpl(GetEnvironmentPointer(), handle,
                                         qryJavaRef);

  return 0;
}
}  // namespace cache
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb
