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

#ifndef __ES_HELPER_H__
#define __ES_HELPER_H__

#include "es_types.h"

#ifdef __cplusplus
// C++ interface
std::string GetClientEncoding(void* conn);
bool SetClientEncoding(void* conn, std::string& encoding);
ESResult* ESGetResult(void* conn);
void ESClearResult(ESResult* es_result);
void* ConnectDBParams(runtime_options& rt_opts);
std::string GetServerVersion(void* conn);
std::string GetClusterName(void* conn);
std::string GetErrorMsg(void* conn);
ConnErrorType GetErrorType(void* conn);
std::vector< std::string > GetColumnsWithSelectQuery(
    void* conn, const std::string table_name);

// C Interface
extern "C" {
#endif
void XPlatformInitializeCriticalSection(void** critical_section_helper);
void XPlatformEnterCriticalSection(void* critical_section_helper);
void XPlatformLeaveCriticalSection(void* critical_section_helper);
void XPlatformDeleteCriticalSection(void** critical_section_helper);
ConnStatusType Status(void* conn);
int ESExecDirect(void* conn, const char* statement, const char* fetch_size);
void SendCursorQueries(void* conn, const char* cursor);
void Disconnect(void* conn);
void StopRetrieval(void* conn);
#ifdef __cplusplus
}
#endif

#endif  // __ES_HELPER_H__
