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

#include <documentdb/odbc/impl/compute/compute_impl.h>

using namespace documentdb::odbc::common::concurrent;

namespace documentdb {
namespace odbc {
namespace impl {
namespace compute {
ComputeImpl::ComputeImpl(SharedPointer< IgniteEnvironment > env,
                         cluster::SP_ClusterGroupImpl clusterGroup)
    : InteropTarget(env, clusterGroup.Get()->GetComputeProcessor()),
      clusterGroup(clusterGroup) {
  // No-op.
}

bool ComputeImpl::ProjectionContainsPredicate() const {
  return clusterGroup.IsValid() && clusterGroup.Get()->GetPredicate() != 0;
}

std::vector< documentdb::odbc::cluster::ClusterNode > ComputeImpl::GetNodes() {
  return clusterGroup.Get()->GetNodes();
}
}  // namespace compute
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb
