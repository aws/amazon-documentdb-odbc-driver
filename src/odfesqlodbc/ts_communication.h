/*
 * Copyright <2019> Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <queue>
#include <future>
#include <regex>
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
#include <string>
#include <aws/timestream-query/TimestreamQueryClient.h>
// clang-format on

class TSCommunication : public Communication {
   public:
    TSCommunication();

    // Create function for factory
    std::string GetErrorPrefix() override;
    bool ConnectionOptions(runtime_options& rt_opts, bool use_defaults,
                           int expand_dbname, unsigned int option_count) override;
    void DropDBConnection() override;
    int ExecDirect(const char* query, const char* fetch_size_) override;
    void SendCursorQueries(std::string cursor) override;
    ESResult* PopResult() override;
    std::string GetServerVersion() override;
    std::string GetClusterName() override;
    std::shared_ptr< Aws::Http::HttpResponse > IssueRequest(
        const std::string& endpoint, const Aws::Http::HttpMethod request_type,
        const std::string& content_type, const std::string& query,
        const std::string& fetch_size = "", const std::string& cursor = "") override;
    void AwsHttpResponseToString(
        std::shared_ptr< Aws::Http::HttpResponse > response,
        std::string& output) override;
    void SendCloseCursorRequest(const std::string& cursor);
    void StopResultRetrieval() override;
    std::vector< std::string > GetColumnsWithSelectQuery(
        const std::string table_name) override;

   protected:
    void InitializeConnection();
    bool CheckConnectionOptions() override;
    bool EstablishConnection() override;
    //void ConstructESResult(ESResult& result);
    //void GetJsonSchema(ESResult& es_result);
    //void PrepareCursorResult(ESResult& es_result);
    //std::shared_ptr< ErrorDetails > ParseErrorResponse(ESResult& es_result);
   private:

    //bool m_is_retrieving;
    //ESResultQueue m_result_queue;
    runtime_options m_rt_opts;

    //std::string m_response_str;

    std::shared_ptr< Aws::TimestreamQuery::TimestreamQueryClient > m_client;
};

#endif
