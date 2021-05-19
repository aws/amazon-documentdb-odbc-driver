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

#ifndef COMMUNICATION
#define COMMUNICATION

#include <aws/core/Aws.h>

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "es_types.h"
#include "ts_prefetch_queue.h"

/**
 * Communication interface between ODBC library and actual database implementation
 */
class Communication {
   public:
    /**
     * Supported encodings list
     * Currently we support UTF8 only
     */
    static const inline std::vector< std::string > m_supported_client_encodings = {"UTF8"};
    /**
     * Default constructor
     */
    Communication();
    /**
     * Destructor
     */
    virtual ~Communication();
    /**
     * Validate options
     * @param options const runtime_options&
     * @return bool
     */
    virtual bool Validate(const runtime_options& options) = 0;
    /**
     * Connect
     * @param options const runtime_options&
     * @return bool
     */
    virtual bool Connect(const runtime_options& options) = 0;
    /**
     * Disconnect
     */
    virtual void Disconnect() = 0;
    /**
     * Execute query
     * @param stmt StatementClass *
     * @param query const char*
     * @return bool
     */
    virtual bool ExecDirect(StatementClass* stmt, const char* query) = 0;
    /**
     * Cancel query
     * @param stmt StatementClass *
     * @return bool
     */
    virtual bool CancelQuery(StatementClass* stmt) = 0;
    /**
     * Get version
     * @return std::string
     */
    virtual std::string GetVersion() = 0;
    /**
     * Setup
     * @param options const runtime_options&
     * @return bool
     */
    virtual bool Setup(const runtime_options& options);


    // Pending for refactor
    /**
     * Get error prefix
     * @return std::string
     */
    virtual std::string GetErrorPrefix() = 0;
    /**
     * For prefetch mechanism
     * During retrieving data from database, caller can call SQLFreeStmt / SQLCloseCursor to stop it 
     * @param stmt StatementClass*
     */
    virtual void StopResultRetrieval(StatementClass* stmt) = 0;
    /**
     * Get client encoding
     * @return std::string
     */
    virtual std::string GetClientEncoding();
    /**
     * Set client encoding
     * @param encoding const std::string&
     * @return bool
     */
    virtual bool SetClientEncoding(const std::string& encoding);
    /**
     * Log messages
     * @param level LogLevel
     * @param msg const char*
     */
    void LogMsg(LogLevel level, const char* msg);
    /**
     * Get connection status
     * @return ConnStatusType
     */
    ConnStatusType GetStatus();
    /**
     * Get the prefetch queue
     * @return stmt StatementClass*
     */
    PrefetchQueue* GetPrefetchQueue(StatementClass* stmt);
   protected:
    /**
     * Connection status
     */
    ConnStatusType m_status;
    /**
     * Current client encoding
     */
    std::string m_client_encoding;
    /**
     * AWS sdk options
     */
    Aws::SDKOptions m_sdk_options;
    /**
     * Map storing prefetch queues
     */
    std::unordered_map< StatementClass*, std::shared_ptr<PrefetchQueue> > prefetch_queues_map;
};

#endif
