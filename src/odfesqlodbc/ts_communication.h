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

#ifndef TS_COMMUNICATION
#define TS_COMMUNICATION

// clang-format off
#include <memory>
#include <string>
#include "communication.h"
#include "es_types.h"
#include "es_result_queue.h"

//Keep rabbit at top otherwise it gives build error because of some variable names like max, min
#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __APPLE__

#ifdef __APPLE__
#pragma clang diagnostic pop
#endif // __APPLE__
// clang-format on
#include <aws/timestream-query/TimestreamQueryClient.h>

/**
 * AWS Timestream communication class
 */
class TSCommunication : public Communication {
   public:
    /**
     * Default constructor
     */
    TSCommunication();
    /**
     * Setup connection options
     * @param rt_opts runtime_options&
     * @return bool
     */
    virtual bool ConnectionOptions(runtime_options& rt_opts) override;
    /**
     * Drop database connection
     */
    virtual void DropDBConnection() override;
    /**
     * Execute query
     * @param query const char*
     * @param fetch_size const char*
     * @return int
     */
    virtual int ExecDirect(const char* query, const char* fetch_size) override;
    /**
     * Get columns using select query
     * @param table_name const std::string&
     * @return std::vector<std::string>
     */
    virtual std::vector< std::string > GetColumnsWithSelectQuery(
        const std::string& table_name) override;
    /**
     * Get error prefix
     * @return std::string
     */
    virtual std::string GetErrorPrefix() override;
    /**
     * Get server version
     * @return std::string
     */
    virtual std::string GetServerVersion() override;
    /**
     * Get cluster name
     * @return std::string
     */
    virtual std::string GetClusterName() override;
    /**
     * Pop result
     * @return ESResult*
     */
    virtual ESResult* PopResult() override;
    /**
     * Send cursor queries
     * @param cursor const std::string&
     */
    virtual void SendCursorQueries(const std::string& cursor) override;
    /**
     * Send request to close cursor
     * @param cursor const std::string&
     */
    virtual void SendCloseCursorRequest(const std::string& cursor) override;
    /**
     * Stop retrieving results
     */
    virtual void StopResultRetrieval() override;
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
        const std::string& fetch_size = "", const std::string& cursor = "") override;
    /**
     * Convert Aws::Http::HttpResponse to std::string
     * @param response std::shared_ptr< Aws::Http::HttpResposne >
     * @return output std::string
     */
    virtual void AwsHttpResponseToString(
        std::shared_ptr< Aws::Http::HttpResponse > response,
        std::string& output) override;
   protected:
    /**
     * Validate connection options
     * @return bool
     */
    virtual bool CheckConnectionOptions() override;
    /**
     * Estabilish connection
     * @return bool
     */
    virtual bool EstablishConnection() override;
    /**
     * Initialize connection
     */
    void InitializeConnection();
private:
    /**
     * Runtime options
     */
    runtime_options m_rt_opts;
    /**
     * Timestream query client
     */
    std::unique_ptr<Aws::TimestreamQuery::TimestreamQueryClient> m_client;
};

#endif
