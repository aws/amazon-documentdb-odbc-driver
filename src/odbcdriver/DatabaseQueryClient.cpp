/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/core/auth/AWSAuthSigner.h>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include <aws/core/client/CoreErrors.h>
#include <aws/core/client/RetryStrategy.h>
#include <aws/core/http/HttpClient.h>
#include <aws/core/http/HttpClientFactory.h>
#include <aws/core/http/HttpResponse.h>
#include <aws/core/platform/Environment.h>
#include <aws/core/utils/DNS.h>
#include <aws/core/utils/Outcome.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/logging/LogMacros.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <aws/core/utils/threading/Executor.h>
#include "DatabaseQueryClient.h"
#include "DatabaseQueryEndpoint.h"
#include "DatabaseQueryErrorMarshaller.h"
#include "CancelQueryRequest.h"
#include "DescribeEndpointsRequest.h"
#include "QueryRequest.h"

using namespace Aws;
using namespace Aws::Auth;
using namespace Aws::Client;
using namespace Aws::Http;
using namespace Aws::Utils::Json;

static const char* SERVICE_NAME = "database";
static const char* ALLOCATION_TAG = "DatabaseQueryClient";

DatabaseQueryClient::DatabaseQueryClient(
    const Client::ClientConfiguration& clientConfiguration)
    : BASECLASS(
        clientConfiguration,
        Aws::MakeShared< AWSAuthV4Signer >(
            ALLOCATION_TAG,
            Aws::MakeShared< DefaultAWSCredentialsProviderChain >(
                ALLOCATION_TAG),
            SERVICE_NAME,
            Aws::Region::ComputeSignerRegion(clientConfiguration.region)),
        Aws::MakeShared< DatabaseQueryErrorMarshaller >(ALLOCATION_TAG)),
      m_executor(clientConfiguration.executor) {
    init(clientConfiguration);
}

DatabaseQueryClient::DatabaseQueryClient(
    const AWSCredentials& credentials,
    const Client::ClientConfiguration& clientConfiguration)
    : BASECLASS(
        clientConfiguration,
        Aws::MakeShared< AWSAuthV4Signer >(
            ALLOCATION_TAG,
            Aws::MakeShared< SimpleAWSCredentialsProvider >(ALLOCATION_TAG,
                                                            credentials),
            SERVICE_NAME,
            Aws::Region::ComputeSignerRegion(clientConfiguration.region)),
        Aws::MakeShared< DatabaseQueryErrorMarshaller >(ALLOCATION_TAG)),
      m_executor(clientConfiguration.executor) {
    init(clientConfiguration);
}

DatabaseQueryClient::DatabaseQueryClient(
    const std::shared_ptr< AWSCredentialsProvider >& credentialsProvider,
    const Client::ClientConfiguration& clientConfiguration)
    : BASECLASS(
        clientConfiguration,
        Aws::MakeShared< AWSAuthV4Signer >(
            ALLOCATION_TAG, credentialsProvider, SERVICE_NAME,
            Aws::Region::ComputeSignerRegion(clientConfiguration.region)),
        Aws::MakeShared< DatabaseQueryErrorMarshaller >(ALLOCATION_TAG)),
      m_executor(clientConfiguration.executor) {
    init(clientConfiguration);
}

DatabaseQueryClient::~DatabaseQueryClient() {
}

void DatabaseQueryClient::init(const Client::ClientConfiguration& config) {
    SetServiceClientName("Database Query");
    LoadDatabaseQuerySpecificConfig(config);
    m_configScheme = SchemeMapper::ToString(config.scheme);
    if (config.endpointOverride.empty()) {
        m_uri = m_configScheme + "://"
                + DatabaseQueryEndpoint::ForRegion(config.region,
                                                     config.useDualStack);
    } else {
        OverrideEndpoint(config.endpointOverride);
    }
}

void DatabaseQueryClient::LoadDatabaseQuerySpecificConfig(
    const Aws::Client::ClientConfiguration& clientConfiguration) {
    if (!clientConfiguration.endpointOverride.empty()) {
        m_enableEndpointDiscovery = false;
    } else if (clientConfiguration.enableEndpointDiscovery) {
        m_enableEndpointDiscovery =
            clientConfiguration.enableEndpointDiscovery.value();
    } else {
        m_enableEndpointDiscovery = true;

        Aws::String enableEndpointDiscovery =
            Aws::Environment::GetEnv("AWS_ENABLE_ENDPOINT_DISCOVERY");
        if (enableEndpointDiscovery.empty()) {
            enableEndpointDiscovery = Aws::Config::GetCachedConfigValue(
                clientConfiguration.profileName, "endpoint_discovery_enabled");
        }

        if (enableEndpointDiscovery == "true") {
            m_enableEndpointDiscovery = true;
        } else if (enableEndpointDiscovery == "false") {
            m_enableEndpointDiscovery = false;
        } else if (!enableEndpointDiscovery.empty()) {
            AWS_LOGSTREAM_WARN(
                "DatabaseQueryClient",
                R"(Using the SDK default configuration for Endpoint Discovery. )"
                R"(Make sure your environment variable "AWS_ENABLE_ENDPOINT_DISCOVERY" or )"
                R"(your config file's variable "endpoint_discovery_enabled" are explicitly set to "true" or "false" (case-sensitive) or not set at all.)");
        }
    }
}

void DatabaseQueryClient::OverrideEndpoint(const Aws::String& endpoint) {
    if (endpoint.compare(0, 7, "http://") == 0
        || endpoint.compare(0, 8, "https://") == 0) {
        m_uri = endpoint;
    } else {
        m_uri = m_configScheme + "://" + endpoint;
    }
    m_enableEndpointDiscovery = false;
}

CancelQueryOutcome DatabaseQueryClient::CancelQuery(
    const CancelQueryRequest& request) const {
    Aws::Http::URI uri = m_uri;
    if (m_enableEndpointDiscovery) {
        Aws::String endpointKey = "Shared";
        Aws::String endpoint;
        if (m_endpointsCache.Get(endpointKey, endpoint)) {
            AWS_LOGSTREAM_TRACE(
                "CancelQuery",
                "Making request to cached endpoint: " << endpoint);
            uri = m_configScheme + "://" + endpoint;
        } else {
            AWS_LOGSTREAM_TRACE(
                "CancelQuery",
                "Endpoint discovery is enabled and there is no usable endpoint "
                "in cache. Discovering endpoints from service...");
            DescribeEndpointsRequest endpointRequest;
            auto endpointOutcome = DescribeEndpoints(endpointRequest);
            if (endpointOutcome.IsSuccess()
                && !endpointOutcome.GetResult().GetEndpoints().empty()) {
                const auto& item =
                    endpointOutcome.GetResult().GetEndpoints()[0];
                m_endpointsCache.Put(
                    endpointKey, item.GetAddress(),
                    std::chrono::minutes(item.GetCachePeriodInMinutes()));
                uri = m_configScheme + "://" + item.GetAddress();
                AWS_LOGSTREAM_TRACE("CancelQuery",
                                    "Endpoints cache updated. Address: "
                                        << item.GetAddress() << ". Valid in: "
                                        << item.GetCachePeriodInMinutes()
                                        << " minutes. Making request to newly "
                                           "discovered endpoint.");
            } else {
                AWS_LOGSTREAM_ERROR("CancelQuery",
                                    "Failed to discover endpoints "
                                        << endpointOutcome.GetError());
                return CancelQueryOutcome(
                    Aws::Client::AWSError< DatabaseQueryErrors >(
                        DatabaseQueryErrors::RESOURCE_NOT_FOUND,
                        "INVALID_ENDPOINT", "Failed to discover endpoint",
                        false));
            }
        }
    } else {
        Aws::String errorMessage =
            R"(Unable to perform "CancelQuery" without endpoint discovery. )"
            R"(Make sure your environment variable "AWS_ENABLE_ENDPOINT_DISCOVERY", )"
            R"(your config file's variable "endpoint_discovery_enabled" and )"
            R"(ClientConfiguration's "enableEndpointDiscovery" are explicitly set to true or not set at all.)";
        return CancelQueryOutcome(
            Aws::Client::AWSError< DatabaseQueryErrors >(
                DatabaseQueryErrors::INVALID_ACTION, "INVALID_ACTION",
                errorMessage, false));
    }
    return CancelQueryOutcome(MakeRequest(uri, request,
                                          Aws::Http::HttpMethod::HTTP_POST,
                                          Aws::Auth::SIGV4_SIGNER));
}

CancelQueryOutcomeCallable DatabaseQueryClient::CancelQueryCallable(
    const CancelQueryRequest& request) const {
    auto task = Aws::MakeShared< std::packaged_task< CancelQueryOutcome() > >(
        ALLOCATION_TAG,
        [this, request]() { return this->CancelQuery(request); });
    auto packagedFunction = [task]() { (*task)(); };
    m_executor->Submit(packagedFunction);
    return task->get_future();
}

void DatabaseQueryClient::CancelQueryAsync(
    const CancelQueryRequest& request,
    const CancelQueryResponseReceivedHandler& handler,
    const std::shared_ptr< const Aws::Client::AsyncCallerContext >& context)
    const {
    m_executor->Submit([this, request, handler, context]() {
        this->CancelQueryAsyncHelper(request, handler, context);
    });
}

void DatabaseQueryClient::CancelQueryAsyncHelper(
    const CancelQueryRequest& request,
    const CancelQueryResponseReceivedHandler& handler,
    const std::shared_ptr< const Aws::Client::AsyncCallerContext >& context)
    const {
    handler(this, request, CancelQuery(request), context);
}

DescribeEndpointsOutcome DatabaseQueryClient::DescribeEndpoints(
    const DescribeEndpointsRequest& request) const {
    Aws::Http::URI uri = m_uri;
    return DescribeEndpointsOutcome(
        MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST,
                    Aws::Auth::SIGV4_SIGNER));
}

DescribeEndpointsOutcomeCallable
DatabaseQueryClient::DescribeEndpointsCallable(
    const DescribeEndpointsRequest& request) const {
    auto task =
        Aws::MakeShared< std::packaged_task< DescribeEndpointsOutcome() > >(
            ALLOCATION_TAG,
            [this, request]() { return this->DescribeEndpoints(request); });
    auto packagedFunction = [task]() { (*task)(); };
    m_executor->Submit(packagedFunction);
    return task->get_future();
}

void DatabaseQueryClient::DescribeEndpointsAsync(
    const DescribeEndpointsRequest& request,
    const DescribeEndpointsResponseReceivedHandler& handler,
    const std::shared_ptr< const Aws::Client::AsyncCallerContext >& context)
    const {
    m_executor->Submit([this, request, handler, context]() {
        this->DescribeEndpointsAsyncHelper(request, handler, context);
    });
}

void DatabaseQueryClient::DescribeEndpointsAsyncHelper(
    const DescribeEndpointsRequest& request,
    const DescribeEndpointsResponseReceivedHandler& handler,
    const std::shared_ptr< const Aws::Client::AsyncCallerContext >& context)
    const {
    handler(this, request, DescribeEndpoints(request), context);
}

QueryOutcome DatabaseQueryClient::Query(const QueryRequest& request) const {
    Aws::Http::URI uri = m_uri;
    if (m_enableEndpointDiscovery) {
        Aws::String endpointKey = "Shared";
        Aws::String endpoint;
        if (m_endpointsCache.Get(endpointKey, endpoint)) {
            AWS_LOGSTREAM_TRACE(
                "Query", "Making request to cached endpoint: " << endpoint);
            uri = m_configScheme + "://" + endpoint;
        } else {
            AWS_LOGSTREAM_TRACE(
                "Query",
                "Endpoint discovery is enabled and there is no usable endpoint "
                "in cache. Discovering endpoints from service...");
            DescribeEndpointsRequest endpointRequest;
            auto endpointOutcome = DescribeEndpoints(endpointRequest);
            if (endpointOutcome.IsSuccess()
                && !endpointOutcome.GetResult().GetEndpoints().empty()) {
                const auto& item =
                    endpointOutcome.GetResult().GetEndpoints()[0];
                m_endpointsCache.Put(
                    endpointKey, item.GetAddress(),
                    std::chrono::minutes(item.GetCachePeriodInMinutes()));
                uri = m_configScheme + "://" + item.GetAddress();
                AWS_LOGSTREAM_TRACE("Query",
                                    "Endpoints cache updated. Address: "
                                        << item.GetAddress() << ". Valid in: "
                                        << item.GetCachePeriodInMinutes()
                                        << " minutes. Making request to newly "
                                           "discovered endpoint.");
            } else {
                AWS_LOGSTREAM_ERROR("Query", "Failed to discover endpoints "
                                                 << endpointOutcome.GetError());
                return QueryOutcome(
                    Aws::Client::AWSError< DatabaseQueryErrors >(
                        DatabaseQueryErrors::RESOURCE_NOT_FOUND,
                        "INVALID_ENDPOINT", "Failed to discover endpoint",
                        false));
            }
        }
    } else {
        Aws::String errorMessage =
            R"(Unable to perform "Query" without endpoint discovery. )"
            R"(Make sure your environment variable "AWS_ENABLE_ENDPOINT_DISCOVERY", )"
            R"(your config file's variable "endpoint_discovery_enabled" and )"
            R"(ClientConfiguration's "enableEndpointDiscovery" are explicitly set to true or not set at all.)";
        return QueryOutcome(Aws::Client::AWSError< DatabaseQueryErrors >(
            DatabaseQueryErrors::INVALID_ACTION, "INVALID_ACTION",
            errorMessage, false));
    }
    return QueryOutcome(MakeRequest(uri, request,
                                    Aws::Http::HttpMethod::HTTP_POST,
                                    Aws::Auth::SIGV4_SIGNER));
}

QueryOutcomeCallable DatabaseQueryClient::QueryCallable(
    const QueryRequest& request) const {
    auto task = Aws::MakeShared< std::packaged_task< QueryOutcome() > >(
        ALLOCATION_TAG, [this, request]() { return this->Query(request); });
    auto packagedFunction = [task]() { (*task)(); };
    m_executor->Submit(packagedFunction);
    return task->get_future();
}

void DatabaseQueryClient::QueryAsync(
    const QueryRequest& request, const QueryResponseReceivedHandler& handler,
    const std::shared_ptr< const Aws::Client::AsyncCallerContext >& context)
    const {
    m_executor->Submit([this, request, handler, context]() {
        this->QueryAsyncHelper(request, handler, context);
    });
}

void DatabaseQueryClient::QueryAsyncHelper(
    const QueryRequest& request, const QueryResponseReceivedHandler& handler,
    const std::shared_ptr< const Aws::Client::AsyncCallerContext >& context)
    const {
    handler(this, request, Query(request), context);
}
