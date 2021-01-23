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

// clang-format off
#include <memory>
#include "es_types.h"
#include "es_result_queue.h"

//Keep rabbit at top otherwise it gives build error because of some variable names like max, min
#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __APPLE__
#include "rabbit.hpp"
#ifdef __APPLE__
#pragma clang diagnostic pop
#endif // __APPLE__
#include <map>
#include <string>
#include <aws/core/Aws.h>
// clang-format on

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
     * @param query const char*
     * @param fetch_size const char*
     * @return int
     */
    virtual int ExecDirect(const char* query, const char* fetch_size) = 0;
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
     * Get columns using select query
     * @param table_name const std::string&
     * @return std::vector<std::string>
     */
    virtual std::vector< std::string > GetColumnsWithSelectQuery(
        const std::string& table_name) = 0;
    /**
     * Get error prefix
     * @return std::string
     */
    virtual std::string GetErrorPrefix() = 0;
    /**
     * Send request to close cursor
     * @param cursor const std::string&
     */
    virtual void SendCloseCursorRequest(const std::string& cursor) = 0;
    /**
     * Isses a request
     * @param endpoint const std::string&
     * @param request_type const Aws::Http::HttpMethod
     * @param content_type const std::string&
     * @param fetch_size const std::string&
     * @param cursor const std::string&
     * @return std::shared_ptr< Aws::Http::HttpResponse >
     */
    virtual std::shared_ptr< Aws::Http::HttpResponse > IssueRequest(
        const std::string& endpoint, const Aws::Http::HttpMethod request_type,
        const std::string& content_type, const std::string& query,
        const std::string& fetch_size = "", const std::string& cursor = "") = 0;
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
     * Stop retrieving results
     */
    virtual void StopResultRetrieval();
    /**
     * Pop result
     * @return TSResult*
     */
    virtual TSResult* PopResult();
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
     * Indicates if it's still retrieving from the queue
     */
    bool m_is_retrieving;
    /**
     * Result set queue
     */
    ESResultQueue m_result_queue;
};

#endif
