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

#ifndef _DOCUMENTDB_ODBC_CACHE_IMPL
#define _DOCUMENTDB_ODBC_CACHE_IMPL

#include <documentdb/odbc/cache/query/query_scan.h>
#include <documentdb/odbc/cache/query/query_sql.h>
#include <documentdb/odbc/cache/query/query_text.h>
#include <documentdb/odbc/cache/query/query_sql_fields.h>
#include <documentdb/odbc/impl/cache/query/query_impl.h>
#include <documentdb/odbc/impl/cache/query/continuous/continuous_query_impl.h>

#include <documentdb/odbc/impl/interop/interop_target.h>

namespace documentdb {
namespace odbc {
namespace impl {
namespace cache {
namespace query {
namespace continuous {
/* Forward declaration. */
class ContinuousQueryHandleImpl;
}  // namespace continuous
}  // namespace query

/**
 * Cache implementation.
 */
class DOCUMENTDB_IMPORT_EXPORT CacheImpl : private interop::InteropTarget {
 public:
  /**
   * Constructor used to create new instance.
   *
   * @param name Name.
   * @param env Environment.
   * @param javaRef Reference to java object.
   */
  CacheImpl(
      char* name,
      documentdb::odbc::common::concurrent::SharedPointer< IgniteEnvironment > env,
      jobject javaRef);

  /**
   * Destructor.
   */
  ~CacheImpl();

  /**
   * Get name.
   *
   * @return Cache name.
   */
  const char* GetName() const;

  /**
   * Perform ContainsKey.
   *
   * @param inOp Input.
   * @param err Error.
   * @return Result.
   */
  bool ContainsKey(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform ContainsKeys.
   *
   * @param inOp Input.
   * @param err Error.
   * @return Result.
   */
  bool ContainsKeys(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform LocalPeek.
   *
   * @param inOp Input.
   * @param outOp Output.
   * @param err Error.
   */
  void LocalPeek(InputOperation& inOp, OutputOperation& outOp,
                 DocumentDbError& err);

  /**
   * Perform Get.
   *
   * @param inOp Input.
   * @param outOp Output.
   * @param err Error.
   */
  void Get(InputOperation& inOp, OutputOperation& outOp, DocumentDbError& err);

  /**
   * Perform GetAll.
   *
   * @param inOp Input.
   * @param outOp Output.
   * @param err Error.
   */
  void GetAll(InputOperation& inOp, OutputOperation& outOp, DocumentDbError& err);

  /**
   * Perform Put.
   *
   * @param inOp Input.
   * @param err Error.
   */
  void Put(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform PutAll.
   *
   * @param inOp Input.
   * @param err Error.
   */
  void PutAll(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform GetAndPut.
   *
   * @param inOp Input.
   * @param outOp Output.
   * @param err Error.
   */
  void GetAndPut(InputOperation& inOp, OutputOperation& outOp,
                 DocumentDbError& err);

  /**
   * Perform GetAndReplace.
   *
   * @param inOp Input.
   * @param outOp Output.
   * @param err Error.
   */
  void GetAndReplace(InputOperation& inOp, OutputOperation& outOp,
                     DocumentDbError& err);

  /**
   * Perform GetAndRemove.
   *
   * @param inOp Input.
   * @param outOp Output.
   * @param err Error.
   */
  void GetAndRemove(InputOperation& inOp, OutputOperation& outOp,
                    DocumentDbError& err);

  /**
   * Perform PutIfAbsent.
   *
   * @param inOp Input.
   * @param err Error.
   * @return Result
   */
  bool PutIfAbsent(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform GetAndPutIfAbsent.
   *
   * @param inOp Input.
   * @param outOp Output.
   * @param err Error.
   */
  void GetAndPutIfAbsent(InputOperation& inOp, OutputOperation& outOp,
                         DocumentDbError& err);

  /**
   * Perform Replace(K, V).
   *
   * @param inOp Input.
   * @param err Error.
   * @return Result
   */
  bool Replace(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform Replace(K, V, V).
   *
   * @param inOp Input.
   * @param err Error.
   * @return Result
   */
  bool ReplaceIfEqual(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform LocalEvict.
   *
   * @param inOp Input.
   * @param err Error.
   */
  void LocalEvict(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform Clear.
   *
   * @param err Error.
   */
  void Clear(DocumentDbError& err);

  /**
   * Perform Clear.
   *
   * @param inOp Input.
   * @param err Error.
   */
  void Clear(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform ClearAll.
   *
   * @param inOp Input.
   * @param err Error.
   */
  void ClearAll(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform LocalClear.
   *
   * @param inOp Input.
   * @param err Error.
   */
  void LocalClear(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform LocalClearAll.
   *
   * @param inOp Input.
   * @param err Error.
   */
  void LocalClearAll(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform Remove(K).
   *
   * @param inOp Input.
   * @param err Error.
   * @return Result
   */
  bool Remove(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform Remove(K, V).
   *
   * @param inOp Input.
   * @param err Error.
   * @return Result
   */
  bool RemoveIfEqual(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform RemoveAll.
   *
   * @param inOp Input.
   * @param err Error.
   */
  void RemoveAll(InputOperation& inOp, DocumentDbError& err);

  /**
   * Perform RemoveAll.
   *
   * @param err Error.
   */
  void RemoveAll(DocumentDbError& err);

  /**
   * Perform Size.
   *
   * @param peekModes Peek modes.
   * @param local Local flag.
   * @param err Error.
   */
  int32_t Size(int32_t peekModes, bool local, DocumentDbError& err);

  /**
   * Invoke query.
   *
   * @param qry Query.
   * @param err Error.
   * @return Query cursor.
   */
  query::QueryCursorImpl* QuerySql(
      const documentdb::odbc::cache::query::SqlQuery& qry, DocumentDbError& err);

  /**
   * Invoke text query.
   *
   * @param qry Query.
   * @param err Error.
   * @return Query cursor.
   */
  query::QueryCursorImpl* QueryText(
      const documentdb::odbc::cache::query::TextQuery& qry, DocumentDbError& err);

  /**
   * Invoke scan query.
   *
   * @param qry Query.
   * @param err Error.
   * @return Query cursor.
   */
  query::QueryCursorImpl* QueryScan(
      const documentdb::odbc::cache::query::ScanQuery& qry, DocumentDbError& err);

  /**
   * Invoke sql fields query.
   *
   * @param qry Query.
   * @param err Error.
   * @return Query cursor.
   */
  query::QueryCursorImpl* QuerySqlFields(
      const documentdb::odbc::cache::query::SqlFieldsQuery& qry, DocumentDbError& err);

  /**
   * Perform Invoke.
   *
   * @param inOp Input.
   * @param outOp Output.
   * @param err Error.
   */
  void Invoke(InputOperation& inOp, OutputOperation& outOp, DocumentDbError& err);

  /**
   * Perform Invoke of Java entry processor.
   *
   * @param inOp Input.
   * @param outOp Output.
   * @param err Error.
   */
  void InvokeJava(InputOperation& inOp, OutputOperation& outOp,
                  DocumentDbError& err);

  /**
   * Start continuous query execution.
   *
   * @param qry Continuous query.
   * @param err Error.
   * @return Continuous query handle.
   */
  query::continuous::ContinuousQueryHandleImpl* QueryContinuous(
      const common::concurrent::SharedPointer<
          query::continuous::ContinuousQueryImplBase >
          qry,
      DocumentDbError& err);

  /**
   * Start continuous query execution with initial query.
   *
   * @param qry Continuous query.
   * @param initialQry Initial query.
   * @param err Error.
   * @return Continuous query handle.
   */
  query::continuous::ContinuousQueryHandleImpl* QueryContinuous(
      const common::concurrent::SharedPointer<
          query::continuous::ContinuousQueryImplBase >
          qry,
      const documentdb::odbc::cache::query::SqlQuery& initialQry, DocumentDbError& err);

  /**
   * Start continuous query execution with initial query.
   *
   * @param qry Continuous query.
   * @param initialQry Initial query.
   * @param err Error.
   * @return Continuous query handle.
   */
  query::continuous::ContinuousQueryHandleImpl* QueryContinuous(
      const common::concurrent::SharedPointer<
          query::continuous::ContinuousQueryImplBase >
          qry,
      const documentdb::odbc::cache::query::TextQuery& initialQry,
      DocumentDbError& err);

  /**
   * Start continuous query execution with initial query.
   *
   * @param qry Continuous query.
   * @param initialQry Initial query.
   * @param err Error.
   * @return Continuous query handle.
   */
  query::continuous::ContinuousQueryHandleImpl* QueryContinuous(
      const common::concurrent::SharedPointer<
          query::continuous::ContinuousQueryImplBase >
          qry,
      const documentdb::odbc::cache::query::ScanQuery& initialQry,
      DocumentDbError& err);

  /**
   * Executes LocalLoadCache on all cache nodes.
   *
   * @param err Error.
   */
  void LoadCache(DocumentDbError& err);

  /**
   * Loads state from the underlying persistent storage.
   *
   * This method is not transactional and may end up loading a stale value into
   * cache if another thread has updated the value immediately after it has been
   * loaded. It is mostly useful when pre-loading the cache from underlying
   * data store before start, or for read-only caches.
   *
   * @param err Error.
   */
  void LocalLoadCache(DocumentDbError& err);

 private:
  DOCUMENTDB_NO_COPY_ASSIGNMENT(CacheImpl);

  /** Name. */
  char* name;

  /**
   * Internal query execution routine.
   *
   * @param qry Query.
   * @param typ Query type.
   * @param err Error.
   */
  template < typename T >
  query::QueryCursorImpl* QueryInternal(const T& qry, int32_t typ,
                                        DocumentDbError& err);

  /**
   * Start continuous query execution with the initial query.
   *
   * @param qry Continuous query.
   * @param initialQry Initial query to be executed.
   * @param err Error.
   * @return Continuous query handle.
   */
  template < typename T >
  query::continuous::ContinuousQueryHandleImpl* QueryContinuous(
      const common::concurrent::SharedPointer<
          query::continuous::ContinuousQueryImplBase >
          qry,
      const T& initialQry, int32_t typ, int32_t cmd, DocumentDbError& err);
};
}  // namespace cache
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb

#endif
