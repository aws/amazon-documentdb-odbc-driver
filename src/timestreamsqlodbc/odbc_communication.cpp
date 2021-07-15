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

#include "odbc_communication.h"

// timestreamsqlodbc needs to be included before mylog, otherwise mylog will generate
// compiler warnings
// clang-format off
#include "odbc.h"
#include "odbc_statement.h"
#include "aad_credentials_provider.h"
#include "okta_credentials_provider.h"
#include "version.h"
#include "mylog.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/client/DefaultRetryStrategy.h>
#include <aws/timestream-query/model/CancelQueryRequest.h>
#include <aws/timestream-query/model/QueryRequest.h>
// clang-format on

namespace {
    /**
     * A helper class to initialize/shutdown AWS API once per DLL load/unload.
     */
    class AwsSdkHelper {
      public:
        AwsSdkHelper() :
          m_reference_count(0) {
        }

        AwsSdkHelper& operator++() {
          if (1 == ++m_reference_count) {
            std::scoped_lock lock(m_mutex);
            Aws::InitAPI(m_sdk_options);
          }
          return *this;
        }

        AwsSdkHelper& operator--() {
          if (0 == --m_reference_count) {
            std::scoped_lock lock(m_mutex);
            Aws::ShutdownAPI(m_sdk_options);
          }
          return *this;
        }

        Aws::SDKOptions m_sdk_options;
        std::atomic<int> m_reference_count;
        std::mutex m_mutex;
    };

    AwsSdkHelper AWS_SDK_HELPER;

    const Aws::String UA_ID_PREFIX = Aws::String("ts-odbc.");
    const std::string DEFAULT_CREATOR_TYPE = "DEFAULT";

    typedef std::function< std::unique_ptr< Aws::TimestreamQuery::TimestreamQueryClient >(
        const runtime_options& options,
        const Aws::Client::ClientConfiguration& config) > QueryClientCreator;

    QueryClientCreator default_creator =
        [](const runtime_options& ,
           const Aws::Client::ClientConfiguration& config) {
            return std::make_unique<
                Aws::TimestreamQuery::TimestreamQueryClient >(config);
        };

    QueryClientCreator profile =
        [](const runtime_options& options,
           const Aws::Client::ClientConfiguration& config) {
        auto cp = std::make_shared<
            Aws::Auth::ProfileConfigFileAWSCredentialsProvider >(
            options.auth.profile_name.c_str());
        return std::make_unique<
            Aws::TimestreamQuery::TimestreamQueryClient >(cp, config);
    };

    QueryClientCreator iam =
        [](const runtime_options& options,
           const Aws::Client::ClientConfiguration& config) {
        Aws::Auth::AWSCredentials credentials(
            options.auth.uid, options.auth.pwd, options.auth.session_token);
        return std::make_unique< Aws::TimestreamQuery::TimestreamQueryClient >(
            credentials, config);
    };

    QueryClientCreator aad =
        [](const runtime_options& options,
           const Aws::Client::ClientConfiguration& config) {
        auto credential_provider =
            std::make_unique< AADCredentialsProvider >(options.auth);
        return std::make_unique< Aws::TimestreamQuery::TimestreamQueryClient >(credential_provider->GetAWSCredentials(), config);
    };

    QueryClientCreator okta =
        [](const runtime_options& options,
           const Aws::Client::ClientConfiguration& config) {
        auto credential_provider =
            std::make_unique< OktaCredentialsProvider >(options.auth);
        return std::make_unique< Aws::TimestreamQuery::TimestreamQueryClient >(
            credential_provider->GetAWSCredentials(), config);
    };

    std::unordered_map< std::string, QueryClientCreator > creators = {
        {DEFAULT_CREATOR_TYPE, default_creator},
        {AUTHTYPE_AWS_PROFILE, profile},
        {AUTHTYPE_IAM, iam},
        {AUTHTYPE_AAD, aad},
        {AUTHTYPE_OKTA, okta},
    };
}

TSCommunication::TSCommunication() {
  ++AWS_SDK_HELPER;
}

TSCommunication::~TSCommunication() {
  --AWS_SDK_HELPER;
}

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

std::unique_ptr< Aws::TimestreamQuery::TimestreamQueryClient > TSCommunication::CreateQueryClient(const runtime_options& options) {
    Aws::Client::ClientConfiguration config;
    config.userAgent = GetUserAgent();
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
        long max_retry_count_client =
            std::stol(options.conn.max_retry_count_client);
        if (max_retry_count_client < 0) {
            throw std::invalid_argument(
                "Max retry count client cannot be negative.");
        }
        config.retryStrategy =
            std::make_shared< Aws::Client::DefaultRetryStrategy >(
                max_retry_count_client);
    }
    auto creator_type = options.auth.auth_type;
    if (options.auth.auth_type == AUTHTYPE_AWS_PROFILE && options.auth.profile_name.empty()) {
        creator_type = DEFAULT_CREATOR_TYPE;
    }
    auto creator = creators.find(creator_type);
    if (creator == creators.end()) {
        throw std::runtime_error("Unknown auth type: "
                                 + options.auth.auth_type);
    } else {
        return creator->second(options, config);
    }
}

bool TSCommunication::TestQueryClient() {
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

bool TSCommunication::Connect(const runtime_options& options) {
    m_client = CreateQueryClient(options);
    if (m_client == nullptr) {
        throw std::runtime_error("Unable to create TimestreamQueryClient.");
    }
    return TestQueryClient();
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
        prefetch_queues_map[stmt]->Reset();
    }
}

Aws::String TSCommunication::GetUserAgent() {
    Aws::String program_name(GetExeProgramName());
    Aws::String name_suffix = " [" + program_name + "]";
    Aws::String msg = "Name of the application using the driver: " + name_suffix;
    LogMsg(LOG_INFO, msg.c_str());
    return UA_ID_PREFIX + GetVersion() + name_suffix;
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
        if (outcome.IsSuccess() && !outcome.GetResult().GetNextToken().empty()) {
            // Update the query id in statement class if different
            if (sc->query_id == NULL
                || strcmp(sc->query_id, outcome.GetResult().GetQueryId().c_str()) != 0) {
                SC_UnsetQueryId(sc);
                sc->query_id = strdup(outcome.GetResult().GetQueryId().c_str());
            }
            Aws::TimestreamQuery::Model::QueryRequest next_request(request);
            std::promise<Aws::TimestreamQuery::Model::QueryOutcome > next_promise;
            auto success = p->GetPrefetchQueue()->Push(next_promise.get_future());
            if (success) {
                // Issue next request
                client->QueryAsync(next_request.WithNextToken(outcome.GetResult().GetNextToken()),
                    QueryCallback,
                    std::make_shared< Context >(p->GetPrefetchQueue(), p->GetStatement(), std::move(next_promise)));
            }
        } else {
            // End of query
            SC_UnsetQueryId(sc);
        }
        // Made promise
        p->MakePromise(outcome);
        p->GetPrefetchQueue()->NotifyOne();
    }
}

bool TSCommunication::ExecDirect(StatementClass* sc, const char* query) {
    CSTR func = "ExecDirect";
    std::string statement(query);
    std::string msg = "Attempting to execute a query \"" + statement + "\"";
    LogMsg(LOG_DEBUG, msg.c_str());
    if (prefetch_queues_map.find(sc) == prefetch_queues_map.end()) {
        prefetch_queues_map[sc] = std::make_shared<PrefetchQueue>();
    }
    PrefetchQueue* pPrefetchQueue = prefetch_queues_map[sc].get();
    pPrefetchQueue->Reset();
    // Issue request
    Aws::TimestreamQuery::Model::QueryRequest request;
    request.SetQueryString(statement.c_str());
    // Use QueryAsync
    std::promise< Aws::TimestreamQuery::Model::QueryOutcome > promise;
    pPrefetchQueue->SetRetrieving(true);
    auto success = pPrefetchQueue->Push(promise.get_future());
    if (success) {
        m_client->QueryAsync(request, QueryCallback,
                             std::make_shared< Context >(pPrefetchQueue, sc,
                                                         std::move(promise)));
        return true;
    } else {
        // Has been cancelled
        SC_set_error(sc, STMT_OPERATION_CANCELLED, "Operation cancelled",
                     func);
        return false;
    }
}

bool TSCommunication::CancelQuery(StatementClass* stmt) {
    if (stmt == nullptr)
        return false;
    auto prefetch_queue_iterator = prefetch_queues_map.find(stmt);
    if (prefetch_queue_iterator != prefetch_queues_map.end()) {
        auto pPrefetchQueue = prefetch_queue_iterator->second;
        pPrefetchQueue->Reset();
    }
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
