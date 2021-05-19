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

#include "ts_communication.h"

// odfesqlodbc needs to be included before mylog, otherwise mylog will generate
// compiler warnings
// clang-format off
#include "es_odbc.h"
#include "es_statement.h"
#include "aad_credentials_provider.h"
#include "okta_credentials_provider.h"
#include "version.h"
#include "mylog.h"
#include <memory>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/client/DefaultRetryStrategy.h>
#include <aws/timestream-query/model/CancelQueryRequest.h>
#include <aws/timestream-query/model/QueryRequest.h>
// clang-format on

bool TSCommunication::Validate(const runtime_options& options) {
    if (options.auth.region.empty() && options.auth.end_point_override.empty()) {
        throw std::invalid_argument("Both region and end point cannot be empty.");
    }
    if (options.auth.auth_type != AUTHTYPE_AWS_PROFILE &&
        options.auth.auth_type != AUTHTYPE_IAM &&
        options.auth.auth_type != AUTHTYPE_AAD &&
        options.auth.auth_type != AUTHTYPE_OKTA) {
        throw std::invalid_argument("Unknown authentication type: \"" + options.auth.auth_type + "\".");
    }
    if (options.auth.auth_type != AUTHTYPE_AWS_PROFILE && options.auth.uid.empty()) {
        throw std::invalid_argument("UID / AccessKeyId cannot be empty.");
    }
    if (options.auth.auth_type != AUTHTYPE_AWS_PROFILE && options.auth.pwd.empty()) {
        throw std::invalid_argument("PWD / SecretAccessKey cannot be empty.");
    }
    LogMsg(LOG_DEBUG, "Required connection options are valid.");
    return true;
}

bool TSCommunication::Connect(const runtime_options& options) {
    Aws::Client::ClientConfiguration config;
    if (!options.auth.end_point_override.empty()) {
        config.endpointOverride = options.auth.end_point_override;
    } else {
        config.enableEndpointDiscovery = true;
        config.region = options.auth.region;
    }
    long request_timeout = static_cast< long >(DEFAULT_REQUEST_TIMEOUT);
    if (!options.conn.timeout.empty()) {
        request_timeout = std::stol(options.conn.timeout);
    }
    if (request_timeout >= 0) {
        config.requestTimeoutMs = request_timeout;
    }
    long connection_timeout = static_cast< long >(DEFAULT_CONNECTION_TIMEOUT);
    if (!options.conn.connection_timeout.empty()) {
        connection_timeout = std::stol(options.conn.connection_timeout);
    }
    if (connection_timeout >= 0) {
        config.connectTimeoutMs = connection_timeout;
    }
    int max_connections = static_cast< int >(DEFAULT_MAX_CONNECTIONS);
    if (!options.conn.max_connections.empty()) {
        max_connections = std::stoi(options.conn.max_connections);
    }
    if (max_connections >= 0) {
        config.maxConnections = max_connections;
    }
    if (!options.conn.max_retry_count_client.empty()) {
        long max_retry_count_client = std::stol(options.conn.max_retry_count_client);
        if (max_retry_count_client < 0) {
            throw std::invalid_argument("Max retry count client cannot be negative.");
        }
        config.retryStrategy = std::make_shared< Aws::Client::DefaultRetryStrategy >(max_retry_count_client);
    }
    if (options.auth.auth_type == AUTHTYPE_AWS_PROFILE) {
        if (!options.auth.profile_name.empty()) {
            auto cp = std::make_shared<Aws::Auth::ProfileConfigFileAWSCredentialsProvider>(options.auth.profile_name.c_str());
            m_client =
                std::make_unique< Aws::TimestreamQuery::TimestreamQueryClient >(cp, config);
        } else {
            m_client =
                std::make_unique< Aws::TimestreamQuery::TimestreamQueryClient >(config);
        }
    } else if (options.auth.auth_type == AUTHTYPE_IAM) {
        Aws::Auth::AWSCredentials credentials(options.auth.uid,
                                              options.auth.pwd, options.auth.session_token);
        m_client =
            std::make_unique< Aws::TimestreamQuery::TimestreamQueryClient >(
                credentials, config);
    } else if (options.auth.auth_type == AUTHTYPE_AAD) {
        m_client = CreateQueryClientWithIdp(
            std::make_unique< AADCredentialsProvider >(options.auth), config);
    } else if (options.auth.auth_type == AUTHTYPE_OKTA) {
        m_client = CreateQueryClientWithIdp(
            std::make_unique< OktaCredentialsProvider >(options.auth), config);
    } else {
        throw std::runtime_error("Unknown auth type: " + options.auth.auth_type);
    }

    if (m_client == nullptr) {
        throw std::runtime_error("Unable to create TimestreamQueryClient.");
    }

    Aws::TimestreamQuery::Model::QueryRequest req;
    req.SetQueryString("select 1");
    auto outcome = m_client->Query(req);
    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError().GetMessage();
        LogMsg(LOG_ERROR, err.c_str());
        Disconnect();
        throw std::runtime_error("Failed to establish connection: " + err);
    }
    LogMsg(LOG_DEBUG, "Connection Established.");
    return true;
}

void TSCommunication::Disconnect() {
    LogMsg(LOG_DEBUG, "Disconnecting Timestream connection.");
    if (m_client) {
        m_client.reset();
    }
    m_status = ConnStatusType::CONNECTION_BAD;
    for (auto& [stmt, prefetchQueue] : prefetch_queues_map) {
        StopResultRetrieval(stmt);
    }
}

std::string TSCommunication::GetVersion() {
    return TIMESTREAMDRIVERVERSION;
}

std::string TSCommunication::GetErrorPrefix() {
    return "[Timestream][SQL ODBC Driver] ";
}

void TSCommunication::StopResultRetrieval(StatementClass* stmt) {
    // Call Cancel logic
    CancelQuery(stmt);
    // Clean the queue
    if (stmt != nullptr && prefetch_queues_map.find(stmt) != prefetch_queues_map.end()) {
        prefetch_queues_map[stmt]->Clear();
        prefetch_queues_map.erase(stmt);
    }
}

std::unique_ptr< Aws::TimestreamQuery::TimestreamQueryClient >
TSCommunication::CreateQueryClientWithIdp(
    std::unique_ptr< SAMLCredentialsProvider > cp,
    const Aws::Client::ClientConfiguration& config) {
    auto credentials = cp->GetAWSCredentials();
    auto query_client =
        std::make_unique< Aws::TimestreamQuery::TimestreamQueryClient >(
            credentials, config);
    return query_client;
}

/**
 * Context class for Aws::Client::AsyncCallerContext
 * Only for execution
 */
class Context : public Aws::Client::AsyncCallerContext {
   public:
    /**
     * Parameterized constructor for the context
     */
    Context(PrefetchQueue* q, StatementClass* s, std::promise< Aws::TimestreamQuery::Model::QueryOutcome > p)
        : Aws::Client::AsyncCallerContext(), queue_(q), stmt_(s), promise_(std::move(p)) {
    }
    /**
     * Get the prefetch queue
     * @return PrefetchQueue*
     */
    PrefetchQueue* GetPrefetchQueue() {
        return queue_;
    }
    /**
     * Get the statement
     * @return StatementClass*
     */
    StatementClass* GetStatement() {
        return stmt_;
    }
    /**
     * Make promise
     * @param outcome const Aws::TimestreamQuery::Model::QueryOutcome&
     */
    void MakePromise(const Aws::TimestreamQuery::Model::QueryOutcome& outcome) {
        promise_.set_value(outcome);
    }
   private:
    /**
     * PrefetchQueue pointer
     */
    PrefetchQueue* queue_;
    /**
     * Statement pointer
     */
    StatementClass* stmt_;
    /**
     * Promise made by the request
     * Wait to be fullfilled in the QueryCallback function
     */
    std::promise< Aws::TimestreamQuery::Model::QueryOutcome > promise_;
};

// Callback function of QueryAsync operation by aws-sdk-cpp timestream-query
void QueryCallback(
    const Aws::TimestreamQuery::TimestreamQueryClient* client,
    const Aws::TimestreamQuery::Model::QueryRequest& request,
    const Aws::TimestreamQuery::Model::QueryOutcome& outcome,
    const std::shared_ptr< const Aws::Client::AsyncCallerContext >& context) {
    auto ctxt = (std::static_pointer_cast< const Context >(context));
    auto p = const_cast< Context* >(ctxt.get());
    auto sc = p->GetStatement();
    if (p != nullptr && sc != nullptr) {
        if (outcome.IsSuccess()) {
            if (!outcome.GetResult().GetNextToken().empty() && sc->retrieving) {
                // Update the query id in statement class if different
                if (sc->query_id == NULL
                    || strcmp(sc->query_id,
                              outcome.GetResult().GetQueryId().c_str())
                           != 0) {
                    SC_UnsetQueryId(sc);
                    sc->query_id =
                        strdup(outcome.GetResult().GetQueryId().c_str());
                }
                // Issue next request
                Aws::TimestreamQuery::Model::QueryRequest next_request(request);
                std::promise< Aws::TimestreamQuery::Model::QueryOutcome >
                    next_promise;
                if (!p->GetPrefetchQueue()->IsEmpty()) {
                    PrefetchQueue* pPrefetchQueue =
                        (PrefetchQueue*)p->GetPrefetchQueue();
                    pPrefetchQueue->Push(next_promise.get_future());
                    client->QueryAsync(next_request.WithNextToken(
                                           outcome.GetResult().GetNextToken()),
                                       QueryCallback,
                                       std::make_shared< Context >(
                                           pPrefetchQueue, p->GetStatement(),
                                           std::move(next_promise)));
                }
            } else {
                // End of query
                sc->retrieving = FALSE;
                SC_UnsetQueryId(sc);
            }
        } else {
            sc->retrieving = FALSE;
        }
        // Made promise from previous query
        std::mutex* mx = static_cast< std::mutex* >(sc->cv_mutex);
        std::condition_variable* cv =
            static_cast< std::condition_variable* >(sc->cv);
        if (mx != nullptr && cv != nullptr) {
            {
                std::unique_lock< std::mutex > lock(*mx);
                p->MakePromise(outcome);
            }
            cv->notify_one();
        }
    }
}

bool TSCommunication::ExecDirect(StatementClass* sc, const char* query) {
    // Prepare statement
    std::string statement(query);
    std::string msg = "Attempting to execute a query \"" + statement + "\"";
    LogMsg(LOG_DEBUG, msg.c_str());
    if (prefetch_queues_map.find(sc) == prefetch_queues_map.end()) {
        prefetch_queues_map[sc] = std::make_shared<PrefetchQueue>();
    }
    PrefetchQueue* pPrefetchQueue = prefetch_queues_map[sc].get();
    pPrefetchQueue->Clear();
    // Issue request
    Aws::TimestreamQuery::Model::QueryRequest request;
    request.SetQueryString(statement.c_str());
    // Use QueryAsync
    std::promise< Aws::TimestreamQuery::Model::QueryOutcome > promise;
    pPrefetchQueue->Push(promise.get_future());
    sc->retrieving = TRUE;
    m_client->QueryAsync(
        request, QueryCallback,
        std::make_shared< Context >(pPrefetchQueue, sc, std::move(promise)));
    return true;
}

bool TSCommunication::CancelQuery(StatementClass* stmt) {
    if (stmt == nullptr)
        return false;
    std::mutex* mx = static_cast< std::mutex* >(stmt->cv_mutex);
    std::condition_variable* cv =
        static_cast< std::condition_variable* >(stmt->cv);
    {
        std::unique_lock< std::mutex > lock(*mx);
        // Break the next request
        stmt->retrieving = FALSE;
    }
    // Set condition variables
    cv->notify_one();
    // Try to cancel current query (Not guaranteed in Timestream service)
    if (stmt->query_id != nullptr && strlen(stmt->query_id) != 0) {
        Aws::TimestreamQuery::Model::CancelQueryRequest cancel_request;
        cancel_request.SetQueryId(stmt->query_id);
        auto outcome = m_client->CancelQuery(cancel_request);
        if (outcome.IsSuccess()) {
            std::string message =
                "Query ID: " + cancel_request.GetQueryId() + "is cancelled. "
                + outcome.GetResult().GetCancellationMessage();
            LogMsg(LOG_DEBUG, message.c_str());
        } else {
            std::string message =
                "Query ID: " + cancel_request.GetQueryId() + "can't cancel. "
                                  + outcome.GetError().GetMessage();
            LogMsg(LOG_DEBUG, message.c_str());
        }
        return outcome.IsSuccess();
    }
    return false;
}
