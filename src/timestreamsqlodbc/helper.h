/*
 * Copyright <2021> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */

#ifndef __HELPER_H__
#define __HELPER_H__

#include "types.h"
#ifdef __cplusplus
#include "prefetch_queue.h"
// C++ interface
std::string GetClientEncoding(void* conn);
bool SetClientEncoding(void* conn, std::string& encoding);
void* ConnectDBParams(const runtime_options& rt_opts);
std::string GetVersion(void* conn);
PrefetchQueue* GetPrefetchQueue(void* conn, StatementClass* stmt);

// C Interface
extern "C" {
#endif
void XPlatformInitializeCriticalSection(void** critical_section_helper);
void XPlatformEnterCriticalSection(void* critical_section_helper);
void XPlatformLeaveCriticalSection(void* critical_section_helper);
void XPlatformDeleteCriticalSection(void** critical_section_helper);
ConnStatusType GetStatus(void* conn);
BOOL ExecDirect(void* conn, StatementClass* stmt, const char* statement);
BOOL CancelQuery(StatementClass* stmt);
void Disconnect(void* conn);
void StopRetrieval(void* conn, StatementClass* stmt);
#ifdef __cplusplus
}
#endif

#endif  // __HELPER_H__
