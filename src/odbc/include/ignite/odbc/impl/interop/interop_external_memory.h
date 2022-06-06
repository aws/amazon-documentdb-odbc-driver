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

#ifndef _IGNITE_ODBC_IMPL_INTEROP_INTEROP_EXTERNAL_MEMORY
#define _IGNITE_ODBC_IMPL_INTEROP_INTEROP_EXTERNAL_MEMORY

#include <stdint.h>

#include <ignite/odbc/common/common.h>

#include <ignite/odbc/impl/interop/interop_memory.h>

namespace ignite 
{
    namespace odbc
    {
        namespace impl 
        {
            namespace interop
            {
                /**
                 * Interop external memory.
                 */
                class IGNITE_IMPORT_EXPORT InteropExternalMemory : public InteropMemory
                {
                public:
                    /**
                     * Constructor.
                     *
                     * @param memPtr External memory pointer.
                     */
                    explicit InteropExternalMemory(int8_t* memPtr);
    
                    virtual void Reallocate(int32_t cap);
                private:
                    IGNITE_NO_COPY_ASSIGNMENT(InteropExternalMemory);
                };
            }
        }
    }
}

#endif //_IGNITE_ODBC_IMPL_INTEROP_INTEROP_EXTERNAL_MEMORY
