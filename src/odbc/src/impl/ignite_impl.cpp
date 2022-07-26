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

#include "documentdb/odbc/cluster/cluster_group.h"
#include "documentdb/odbc/impl/ignite_impl.h"

using namespace documentdb::odbc::common::concurrent;
using namespace documentdb::odbc::cache;
using namespace documentdb::odbc::cluster;
using namespace documentdb::odbc::jni::java;
using namespace documentdb::odbc::impl::interop;
using namespace documentdb::odbc::impl::binary;
using namespace documentdb::odbc::impl::cache;
using namespace documentdb::odbc::impl::cluster;

using namespace documentdb::odbc::binary;

namespace documentdb {
namespace odbc {
namespace impl {
/*
 * PlatformProcessor op codes.
 */
struct ProcessorOp {
  enum Type {
    GET_CACHE = 1,
    CREATE_CACHE = 2,
    GET_OR_CREATE_CACHE = 3,
    GET_AFFINITY = 7,
    GET_TRANSACTIONS = 9,
    GET_CLUSTER_GROUP = 10,
    SET_BASELINE_TOPOLOGY_VERSION = 24,
    WAL_DISABLE = 27,
    WAL_ENABLE = 28,
    WAL_IS_ENABLED = 29,
    SET_TX_TIMEOUT_ON_PARTITION_MAP_EXCHANGE = 30,
  };
};

IgniteImpl::IgniteImpl(SharedPointer< IgniteEnvironment > env)
    : InteropTarget(env, static_cast< jobject >(env.Get()->GetProcessor()),
                    true),
      env(env),
      prjImpl() {
  prjImpl.Init(common::Bind(this, &IgniteImpl::InternalGetProjection));
}

IgniteImpl::SP_CacheAffinityImpl IgniteImpl::GetAffinity(
    const std::string& cacheName, DocumentDbError& err) {
  SharedPointer< InteropMemory > mem = env.Get()->AllocateMemory();
  InteropMemory* mem0 = mem.Get();
  InteropOutputStream out(mem0);
  BinaryWriterImpl writer(&out, env.Get()->GetTypeManager());
  BinaryRawWriter rawWriter(&writer);

  rawWriter.WriteString(cacheName);

  out.Synchronize();

  jobject affinityJavaRef =
      InStreamOutObject(ProcessorOp::GET_AFFINITY, *mem0, err);

  if (!affinityJavaRef) {
    return NULL;
  }

  return new CacheAffinityImpl(env, affinityJavaRef);
}

const char* IgniteImpl::GetName() const {
  return env.Get()->InstanceName();
}

const IgniteConfiguration& IgniteImpl::GetConfiguration() const {
  return env.Get()->GetConfiguration();
}

JniContext* IgniteImpl::GetContext() {
  return env.Get()->Context();
}

CacheImpl* IgniteImpl::GetCache(const char* name, DocumentDbError& err) {
  return GetOrCreateCache(name, err, ProcessorOp::GET_CACHE);
}

CacheImpl* IgniteImpl::GetOrCreateCache(const char* name, DocumentDbError& err) {
  return GetOrCreateCache(name, err, ProcessorOp::GET_OR_CREATE_CACHE);
}

CacheImpl* IgniteImpl::CreateCache(const char* name, DocumentDbError& err) {
  return GetOrCreateCache(name, err, ProcessorOp::CREATE_CACHE);
}

IgniteImpl::SP_IgniteBindingImpl IgniteImpl::GetBinding() {
  return env.Get()->GetBinding();
}

IgniteImpl::SP_IgniteClusterImpl IgniteImpl::GetCluster() {
  return IgniteImpl::SP_IgniteClusterImpl(
      new IgniteClusterImpl(this->GetProjection()));
}

IgniteImpl::SP_ComputeImpl IgniteImpl::GetCompute() {
  SP_ClusterGroupImpl serversCluster = prjImpl.Get().Get()->ForServers();

  return GetCompute(serversCluster);
}

IgniteImpl::SP_ComputeImpl IgniteImpl::GetCompute(ClusterGroup grp) {
  SP_ClusterGroupImpl grpImpl = grp.GetImpl();

  return SP_ComputeImpl(
      new compute::ComputeImpl(GetEnvironmentPointer(), grpImpl));
}

void IgniteImpl::DisableWal(std::string cacheName) {
  DocumentDbError err;
  In1Operation< std::string > inOp(cacheName);

  InteropTarget::OutOp(ProcessorOp::WAL_DISABLE, inOp, err);

  DocumentDbError::ThrowIfNeeded(err);
}

void IgniteImpl::EnableWal(std::string cacheName) {
  DocumentDbError err;
  In1Operation< std::string > inOp(cacheName);

  InteropTarget::OutOp(ProcessorOp::WAL_ENABLE, inOp, err);

  DocumentDbError::ThrowIfNeeded(err);
}

bool IgniteImpl::IsWalEnabled(std::string cacheName) {
  DocumentDbError err;
  In1Operation< std::string > inOp(cacheName);

  bool ret = InteropTarget::OutOp(ProcessorOp::WAL_IS_ENABLED, inOp, err);

  DocumentDbError::ThrowIfNeeded(err);

  return ret;
}

void IgniteImpl::SetBaselineTopologyVersion(int64_t topVer) {
  DocumentDbError err;

  OutInOpLong(ProcessorOp::SET_BASELINE_TOPOLOGY_VERSION, topVer, err);

  DocumentDbError::ThrowIfNeeded(err);
}

void IgniteImpl::SetTxTimeoutOnPartitionMapExchange(int64_t timeout) {
  DocumentDbError err;

  if (timeout < 0) {
    const char* msg = "Impossible to set negative timeout";
    throw DocumentDbError(DocumentDbError::IGNITE_ERR_ILLEGAL_ARGUMENT, msg);
  }

  In1Operation< int64_t > inOp(timeout);

  OutOp(ProcessorOp::SET_TX_TIMEOUT_ON_PARTITION_MAP_EXCHANGE, inOp, err);

  DocumentDbError::ThrowIfNeeded(err);
}

ClusterGroupImpl* IgniteImpl::InternalGetProjection() {
  DocumentDbError err;

  jobject clusterGroupJavaRef = InOpObject(ProcessorOp::GET_CLUSTER_GROUP, err);

  DocumentDbError::ThrowIfNeeded(err);

  if (!clusterGroupJavaRef)
    throw DocumentDbError(DocumentDbError::IGNITE_ERR_GENERIC,
                      "Can not get ClusterGroup instance.");

  return new ClusterGroupImpl(env, clusterGroupJavaRef);
}

CacheImpl* IgniteImpl::GetOrCreateCache(const char* name, DocumentDbError& err,
                                        int32_t op) {
  SharedPointer< InteropMemory > mem = env.Get()->AllocateMemory();
  InteropMemory* mem0 = mem.Get();
  InteropOutputStream out(mem0);
  BinaryWriterImpl writer(&out, env.Get()->GetTypeManager());
  BinaryRawWriter rawWriter(&writer);

  rawWriter.WriteString(name);

  out.Synchronize();

  jobject cacheJavaRef = InStreamOutObject(op, *mem0, err);

  if (!cacheJavaRef) {
    return NULL;
  }

  char* name0 = common::CopyChars(name);

  return new CacheImpl(name0, env, cacheJavaRef);
}
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb
