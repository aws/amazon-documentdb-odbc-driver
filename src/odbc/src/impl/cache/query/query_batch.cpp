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

<<<<<<<< HEAD:src/odbc/src/impl/compute/compute_impl.cpp
#include <ignite/odbc/impl/compute/compute_impl.h>

using namespace ignite::odbc::common::concurrent;

namespace ignite
{
    namespace odbc
    {
        namespace impl
        {
            namespace compute
            {
                ComputeImpl::ComputeImpl(SharedPointer<IgniteEnvironment> env, cluster::SP_ClusterGroupImpl clusterGroup) :
                    InteropTarget(env, clusterGroup.Get()->GetComputeProcessor()),
                    clusterGroup(clusterGroup)
                {
                    // No-op.
                }
    
                bool ComputeImpl::ProjectionContainsPredicate() const
                {
                    return clusterGroup.IsValid() && clusterGroup.Get()->GetPredicate() != 0;
                }
    
                std::vector<ignite::odbc::cluster::ClusterNode> ComputeImpl::GetNodes()
                {
                    return clusterGroup.Get()->GetNodes();
                }
            }
        }
    }
========
#include "ignite/odbc/impl/cache/query/query_batch.h"
#include "ignite/odbc/impl/cache/query/query_fields_row_impl.h"

namespace ignite {
namespace odbc {
namespace impl {
namespace cache {
namespace query {
QueryFieldsRowImpl* QueryBatch::GetNextRow() {
  assert(Left() > 0);

  int32_t rowBegin = stream.Position();

  int32_t rowLen = reader.ReadInt32();
  int32_t columnNum = reader.ReadInt32();

  int32_t dataPos = stream.Position();

  assert(rowLen >= 4);

  ++pos;

  stream.Position(rowBegin + rowLen);

  return new QueryFieldsRowImpl(mem, dataPos, columnNum);
>>>>>>>> 7f25089f0c1c376439b2ccaaccc36f5e74b8f9e4:src/odbc/src/impl/cache/query/query_batch.cpp
}

}  // namespace query
}  // namespace cache
}  // namespace impl
}  // namespace odbc
}  // namespace ignite
