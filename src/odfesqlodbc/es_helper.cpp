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

#include "es_helper.h"

#include <atomic>
#include <mutex>
#include <thread>

#include "ts_communication.h"

void* ConnectDBParams(runtime_options& rt_opts) {
    auto conn = new TSCommunication();

    if (!conn)
        return nullptr;

    // Set user defined connection options
    if (!conn->ConnectionOptions(rt_opts))
        return conn;

    // Connect to DB
    if (!conn->ConnectDBStart())
        return conn;

    // Technically this is always the result, so we could remove the above or
    // make 1 large if statement, but I think this is more legible
    return conn;
}

ConnStatusType Status(void* conn) {
    return conn
               ? static_cast< Communication* >(conn)->GetConnectionStatus()
               : ConnStatusType::CONNECTION_BAD;
}

std::string GetErrorMsg(void* conn) {
    return conn ? static_cast< Communication* >(conn)->GetErrorMessage()
                   : NULL;
}

ConnErrorType GetErrorType(void* conn) {
    return conn ? static_cast< Communication* >(conn)->GetErrorType()
                   : ConnErrorType::CONN_ERROR_SUCCESS;
}

std::string GetServerVersion(void* conn) {
    return conn
               ? static_cast< Communication* >(conn)->GetServerVersion()
               : "";
}

std::string GetClusterName(void* conn) {
    return conn ? static_cast< Communication* >(conn)->GetClusterName()
                   : "";
}

int ESExecDirect(void* conn, const char* statement, const char* fetch_size) {
    return (conn && statement)
               ? static_cast< Communication* >(conn)->ExecDirect(
                   statement, fetch_size)
               : -1;
}

void SendCursorQueries(void* conn, const char* cursor) {
    static_cast< Communication* >(conn)->SendCursorQueries(cursor);
}

ESResult* ESGetResult(void* conn) {
    return conn ? static_cast< Communication* >(conn)->PopResult()
                   : NULL;
}

std::string GetClientEncoding(void* conn) {
    return conn
               ? static_cast< Communication* >(conn)->GetClientEncoding()
               : "";
}

bool SetClientEncoding(void* conn, std::string& encoding) {
    return conn
               ? static_cast< Communication* >(conn)->SetClientEncoding(
                   encoding)
               : false;
}

void Disconnect(void* conn) {
    delete static_cast< Communication* >(conn);
}

void ESClearResult(ESResult* es_result) {
    delete es_result;
}

void StopRetrieval(void* conn) {
    static_cast< Communication* >(conn)->StopResultRetrieval();
}

std::vector< std::string > GetColumnsWithSelectQuery(
    void* conn, const std::string& table_name) {
    return static_cast< Communication* >(conn)->GetColumnsWithSelectQuery(
        table_name);
}

// This class provides a cross platform way of entering critical sections
class CriticalSectionHelper {
   public:
    // Don't need to initialize lock owner because default constructor sets it
    // to thread id 0, which is invalid
    CriticalSectionHelper() : m_lock_count(0) {
    }
    ~CriticalSectionHelper() {
    }

    void EnterCritical() {
        // Get current thread id, if it's the lock owner, increment lock count,
        // otherwise lock and take ownership
        std::thread::id current_thread = std::this_thread::get_id();
        if (m_lock_owner == current_thread) {
            m_lock_count++;
        } else {
            m_lock.lock();
            m_lock_owner = current_thread;
            m_lock_count = 1;
        }
    }

    void ExitCritical() {
        // Get current thread id, if it's the owner, decerement and unlock if
        // the lock count is 0. Otherwise, log critical warning because we
        // should only allow the lock owner to unlock
        std::thread::id current_thread = std::this_thread::get_id();
        if (m_lock_owner == current_thread) {
            if (m_lock_count == 0) {
// This should never happen. Log critical warning
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4551)  // MYLOG complains 'function call missing
                                 // argument list' on Windows, which is isn't
#endif
                MYLOG(ES_ERROR, "%s\n",
                      "CRITICAL WARNING: ExitCritical section called when lock "
                      "count was already 0!");
#ifdef WIN32
#pragma warning(pop)
#endif
            } else if (--m_lock_count == 0) {
                // Reset lock owner to invalid thread id (0)
                m_lock_owner = std::thread::id();
                m_lock.unlock();
            }
        } else {
// This should never happen. Log critical warning
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4551)  // MYLOG complains 'function call missing
                                 // argument list' on Windows, which is isn't
#endif
            MYLOG(ES_ERROR, "%s\n",
                  "CRITICAL WARNING: ExitCritical section called by thread "
                  "that does not own the lock!");
#ifdef WIN32
#pragma warning(pop)
#endif
        }
    }

   private:
    size_t m_lock_count;
    std::atomic< std::thread::id > m_lock_owner;
    std::mutex m_lock;
};

// Initialize pointer to point to our helper class
void XPlatformInitializeCriticalSection(void** critical_section_helper) {
    if (critical_section_helper != NULL) {
        try {
            *critical_section_helper = new CriticalSectionHelper();
        } catch (...) {
            *critical_section_helper = NULL;
        }
    }
}

// Call enter critical section
void XPlatformEnterCriticalSection(void* critical_section_helper) {
    if (critical_section_helper != NULL) {
        static_cast< CriticalSectionHelper* >(critical_section_helper)
            ->EnterCritical();
    }
}

// Call exit critical section
void XPlatformLeaveCriticalSection(void* critical_section_helper) {
    if (critical_section_helper != NULL) {
        static_cast< CriticalSectionHelper* >(critical_section_helper)
            ->ExitCritical();
    }
}

// Delete our helper class
void XPlatformDeleteCriticalSection(void** critical_section_helper) {
    if (critical_section_helper != NULL) {
        delete static_cast< CriticalSectionHelper* >(*critical_section_helper);
        *critical_section_helper = NULL;
    }
}
