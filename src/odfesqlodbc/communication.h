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
#include <queue>
#include <future>
#include <regex>
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
     * Setup connection options
     * @param rt_opts runtime_options&
     * @return bool
     */
    virtual bool ConnectionOptions(runtime_options& rt_opts) = 0;
    /**
     * Drop database connection
     */
    virtual void DropDBConnection() = 0;
    /**
     * Execute query
     * @param query const char*
     * @param fetch_size const char*
     * @return int
     */
    virtual int ExecDirect(const char* query, const char* fetch_size) = 0;
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
     * Get server version
     * @return std::string
     */
    virtual std::string GetServerVersion() = 0;
    /**
     * Get cluster name
     * @return std::string
     */
    virtual std::string GetClusterName() = 0;
    /**
     * Pop result
     * @return ESResult*
     */
    virtual ESResult* PopResult() = 0;
    /**
     * Send cursor queries
     * @param cursor const std::string&
     */
    virtual void SendCursorQueries(const std::string& cursor) = 0;
    /**
     * Send request to close cursor
     * @param cursor const std::string&
     */
    virtual void SendCloseCursorRequest(const std::string& cursor) = 0;
    /**
     * Stop retrieving results
     */
    virtual void StopResultRetrieval() = 0;
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
     * Convert Aws::Http::HttpResponse to std::string
     * @param response std::shared_ptr< Aws::Http::HttpResposne >
     * @return output std::string
     */
    virtual void AwsHttpResponseToString(
        std::shared_ptr< Aws::Http::HttpResponse > response,
        std::string& output) = 0;
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
     * @param level ESLogLevel
     * @param msg const char*
     */
    void LogMsg(ESLogLevel level, const char* msg);
    /**
     * Get error message
     * @return std::string
     */
    virtual std::string GetErrorMessage();
    /**
     * Get error type
     * @return ConnErrorType
     */
    virtual ConnErrorType GetErrorType();
    /**
     * Connect database
     * @return bool
     */
    virtual bool ConnectDBStart();
    /**
     * Get connection status
     * @return ConnStatusType
     */
    ConnStatusType GetConnectionStatus();
   protected:
    /**
     * Validate connection options
     * @return bool
     */
    virtual bool CheckConnectionOptions() = 0;
    /**
     * Estabilish connection
     * @return bool
     */
    virtual bool EstablishConnection() = 0;
    /**
     * Set error details
     * @param reason const std::string&
     * @param message const std::string&
     * @param error_type ConnectErrorType
     */
    virtual void SetErrorDetails(const std::string& reason, const std::string& message,
                         ConnErrorType error_type);
    /**
     * Set error details
     * @param details ErrorDetails
     */
    virtual void SetErrorDetails(ErrorDetails details);

    /**
     * Latest error message
     */
    std::string m_error_message;
    /**
     * Latest error details
     */
    std::shared_ptr< ErrorDetails > m_error_details;
    /**
     * Connection status
     */
    ConnStatusType m_status;
    /**
     * Connection error type
     */
    ConnErrorType m_error_type;
    /**
     * Flag to indicate a valid connection options
     */
    bool m_is_valid_connection_options;
    /**
     * Current client encoding
     */
    std::string m_client_encoding;
    /**
     * AWS sdk options
     */
    Aws::SDKOptions m_options;
};

#endif
