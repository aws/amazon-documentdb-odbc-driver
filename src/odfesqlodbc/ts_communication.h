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
#include <aws/timestream-query/TimestreamQueryClient.h>
#include <aws/timestream-query/model/Type.h>
#include <aws/timestream-query/model/ScalarType.h>

//Keep rabbit at top otherwise it gives build error because of some variable names like max, min
#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __APPLE__

#ifdef __APPLE__
#pragma clang diagnostic pop
#endif // __APPLE__
// clang-format on

/**
 * AWS Timestream communication class
 */
class TSCommunication : public Communication {
   public:
    /**
     * Validate options
     * @param options const runtime_options&
     * @return bool
     */
    virtual bool Validate(const runtime_options& options) override;
    /**
     * Connect
     * @param options const runtime_options&
     * @return bool
     */
    virtual bool Connect(const runtime_options& options) override;
    /**
     * Disconnect
     */
    virtual void Disconnect() override;
    /**
     * Execute query
     * @param query const char*
     * @return int
     */
    virtual int ExecDirect(const char* query) override;
    /**
     * Get version
     * @return std::string
     */
    virtual std::string GetVersion() override;

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
     * Send request to close cursor
     * @param cursor const std::string&
     */
    virtual void SendCloseCursorRequest(const std::string& cursor) override;
    /**
     * Isses a request
     * @param endpoint const std::string&
     * @param request_type const Aws::Http::HttpMethod
     * @param content_type const std::string&
     * @param cursor const std::string&
     * @return std::shared_ptr< Aws::Http::HttpResponse >
     */
    virtual std::shared_ptr< Aws::Http::HttpResponse > IssueRequest(
        const std::string& endpoint, const Aws::Http::HttpMethod request_type,
        const std::string& content_type, const std::string& query,
        const std::string& cursor = "") override;
    /**
     * Construct the result set
     * @param result TSResult&
     * @return output std::string
     */
    void ConstructTSResult(TSResult& result);
    /**
     * Send cursor queries
     * @param request Aws::TimestreamQuery::Model::QueryRequest
     * @param cursor Aws::String
     */
    void SendCursorQueries(Aws::TimestreamQuery::Model::QueryRequest request,
                           Aws::String next_token);
    /**
     * Runtime options
     */
    runtime_options m_rt_opts;
    /**
     * Timestream query client
     */
    std::unique_ptr< Aws::TimestreamQuery::TimestreamQueryClient > m_client;
};

#endif
