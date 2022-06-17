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

<<<<<<<< HEAD:src/odbc/src/impl/compute/cancelable_impl.cpp
#include <ignite/odbc/impl/compute/cancelable_impl.h>

using namespace ignite::odbc::common::concurrent;
========
#include <ignite/odbc/ignite_error.h>
#include <ignite/odbc/jni/java.h>

#include "ignite/odbc/impl/interop/interop_external_memory.h"
>>>>>>>> 7f25089f0c1c376439b2ccaaccc36f5e74b8f9e4:src/odbc/src/impl/interop/interop_external_memory.cpp

using namespace ignite::odbc::jni::java;

namespace ignite {
namespace odbc {
namespace impl {
namespace interop {
InteropExternalMemory::InteropExternalMemory(int8_t* memPtr) {
  this->memPtr = memPtr;
}

<<<<<<<< HEAD:src/odbc/src/impl/compute/cancelable_impl.cpp
namespace ignite
{
    namespace odbc
    {
        namespace impl
        {
            namespace compute
            {
                CancelableImpl::CancelableImpl(SharedPointer<IgniteEnvironment> env, jobject javaRef) :
                    InteropTarget(env, javaRef),
                    Cancelable()
                {
                    // No-op.
                }
    
                void CancelableImpl::Cancel()
                {
                    IgniteError err;
    
                    OutInOpLong(Operation::Cancel, 0, err);
    
                    IgniteError::ThrowIfNeeded(err);
                }
            }
        }
    }
}
========
void InteropExternalMemory::Reallocate(int32_t cap) {
  if (JniContext::Reallocate(reinterpret_cast< int64_t >(memPtr), cap) == -1) {
    IGNITE_ERROR_FORMATTED_2(IgniteError::IGNITE_ERR_MEMORY,
                             "Failed to reallocate external memory", "memPtr",
                             PointerLong(), "requestedCapacity", cap)
  }
}
}  // namespace interop
}  // namespace impl
}  // namespace odbc
}  // namespace ignite
>>>>>>>> 7f25089f0c1c376439b2ccaaccc36f5e74b8f9e4:src/odbc/src/impl/interop/interop_external_memory.cpp
