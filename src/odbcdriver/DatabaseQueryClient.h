/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/core/client/AWSClient.h>
#include <aws/core/client/AWSError.h>
#include <aws/core/client/AsyncCallerContext.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/HttpTypes.h>
#include <aws/core/utils/ConcurrentCache.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <string>

#include <functional>
#include <future>

#include "CancelQueryResult.h"
#include "DatabaseQueryErrors.h"
#include "DescribeEndpointsResult.h"
#include "QueryResult.h"

namespace Aws {

namespace Http {
class HttpClient;
class HttpClientFactory;
}  // namespace Http

namespace Utils {
template < typename R, typename E >
class Outcome;
namespace Threading {
class Executor;
}  // namespace Threading
}  // namespace Utils

namespace Auth {
class AWSCredentials;
class AWSCredentialsProvider;
}  // namespace Auth

namespace Client {
class RetryStrategy;
}  // namespace Client
}  // namespace Aws

class CancelQueryRequest;
class DescribeEndpointsRequest;
class QueryRequest;

typedef Aws::Utils::Outcome< CancelQueryResult, DatabaseQueryError >
    CancelQueryOutcome;
typedef Aws::Utils::Outcome< DescribeEndpointsResult, DatabaseQueryError >
    DescribeEndpointsOutcome;
typedef Aws::Utils::Outcome< QueryResult, DatabaseQueryError > QueryOutcome;

typedef std::future< CancelQueryOutcome > CancelQueryOutcomeCallable;
typedef std::future< DescribeEndpointsOutcome >
    DescribeEndpointsOutcomeCallable;
typedef std::future< QueryOutcome > QueryOutcomeCallable;

class DatabaseQueryClient;

typedef std::function< void(
    const DatabaseQueryClient*, const CancelQueryRequest&,
    const CancelQueryOutcome&,
    const std::shared_ptr< const Aws::Client::AsyncCallerContext >&) >
    CancelQueryResponseReceivedHandler;
typedef std::function< void(
    const DatabaseQueryClient*, const DescribeEndpointsRequest&,
    const DescribeEndpointsOutcome&,
    const std::shared_ptr< const Aws::Client::AsyncCallerContext >&) >
    DescribeEndpointsResponseReceivedHandler;
typedef std::function< void(
    const DatabaseQueryClient*, const QueryRequest&,
    const QueryOutcome&,
    const std::shared_ptr< const Aws::Client::AsyncCallerContext >&) >
    QueryResponseReceivedHandler;

/**
 * <p> </p>
 */
class DatabaseQueryClient : public Aws::Client::AWSJsonClient {
   public:
    typedef Aws::Client::AWSJsonClient BASECLASS;

    /**
     * Initializes client to use DefaultCredentialProviderChain, with default
     * http client factory, and optional client config. If client config is not
     * specified, it will be initialized to default values.
     */
    DatabaseQueryClient(
        const Aws::Client::ClientConfiguration& clientConfiguration =
            Aws::Client::ClientConfiguration());

    /**
     * Initializes client to use SimpleAWSCredentialsProvider, with default http
     * client factory, and optional client config. If client config is not
     * specified, it will be initialized to default values.
     */
    DatabaseQueryClient(
        const Aws::Auth::AWSCredentials& credentials,
        const Aws::Client::ClientConfiguration& clientConfiguration =
            Aws::Client::ClientConfiguration());

    /**
     * Initializes client to use specified credentials provider with specified
     * client config. If http client factory is not supplied, the default http
     * client factory will be used
     */
    DatabaseQueryClient(
        const std::shared_ptr< Aws::Auth::AWSCredentialsProvider >&
            credentialsProvider,
        const Aws::Client::ClientConfiguration& clientConfiguration =
            Aws::Client::ClientConfiguration());

    virtual ~DatabaseQueryClient();

    /**
     * <p> Cancels a query that has been issued. Cancellation is guaranteed only
     * if the query has not completed execution before the cancellation request
     * was issued. Because cancellation is an idempotent operation, subsequent
     * cancellation requests will return a <code>CancellationMessage</code>,
     * indicating that the query has already been canceled.
     */
    virtual CancelQueryOutcome CancelQuery(
        const CancelQueryRequest& request) const;

    /**
     * <p> Cancels a query that has been issued. Cancellation is guaranteed only
     * if the query has not completed execution before the cancellation request
     * was issued. Because cancellation is an idempotent operation, subsequent
     * cancellation requests will return a <code>CancellationMessage</code>,
     * indicating that the query has already been canceled.
     *
     * returns a future to the operation so that it can be executed in parallel
     * to other requests.
     */
    virtual CancelQueryOutcomeCallable CancelQueryCallable(
        const CancelQueryRequest& request) const;

    /**
     * <p> Cancels a query that has been issued. Cancellation is guaranteed only
     * if the query has not completed execution before the cancellation request
     * was issued. Because cancellation is an idempotent operation, subsequent
     * cancellation requests will return a <code>CancellationMessage</code>,
     * indicating that the query has already been canceled.
     *
     * Queues the request into a thread executor and triggers associated
     * callback when operation has finished.
     */
    virtual void CancelQueryAsync(
        const CancelQueryRequest& request,
        const CancelQueryResponseReceivedHandler& handler,
        const std::shared_ptr< const Aws::Client::AsyncCallerContext >&
            context = nullptr) const;

    /**
     * <p>DescribeEndpoints returns a list of available endpoints to make
     * Database API calls against. This API is available through both Write
     * and Query.</p> <p>Because Database's SDKs are designed to transparently
     * work with the service's architecture, including the management and
     * mapping of the service endpoints, <i>it is not recommended that you use
     * this API unless</i>:</p> <ul> <li> <p>Your application uses a programming
     * language that does not yet have SDK support</p> </li> <li> <p>You require
     * better control over the client-side implementation</p> </li> </ul> <p>For
     * detailed information on how to use DescribeEndpoints.
     */
    virtual DescribeEndpointsOutcome DescribeEndpoints(
        const DescribeEndpointsRequest& request) const;

    /**
     * <p>DescribeEndpoints returns a list of available endpoints to make
     * Database API calls against. This API is available through both Write
     * and Query.</p> <p>Because Database's SDKs are designed to transparently
     * work with the service's architecture, including the management and
     * mapping of the service endpoints, <i>it is not recommended that you use
     * this API unless</i>:</p> <ul> <li> <p>Your application uses a programming
     * language that does not yet have SDK support</p> </li> <li> <p>You require
     * better control over the client-side implementation</p> </li> </ul> <p>For
     * detailed information on how to use DescribeEndpoints.
     *
     * returns a future to the operation so that it can be executed in parallel
     * to other requests.
     */
    virtual DescribeEndpointsOutcomeCallable DescribeEndpointsCallable(
        const DescribeEndpointsRequest& request) const;

    /**
     * <p>DescribeEndpoints returns a list of available endpoints to make
     * Database API calls against. This API is available through both Write
     * and Query.</p> <p>Because Database's SDKs are designed to transparently
     * work with the service's architecture, including the management and
     * mapping of the service endpoints, <i>it is not recommended that you use
     * this API unless</i>:</p> <ul> <li> <p>Your application uses a programming
     * language that does not yet have SDK support</p> </li> <li> <p>You require
     * better control over the client-side implementation</p> </li> </ul> <p>For
     * detailed information on how to use DescribeEndpoints.
     *
     * Queues the request into a thread executor and triggers associated
     * callback when operation has finished.
     */
    virtual void DescribeEndpointsAsync(
        const DescribeEndpointsRequest& request,
        const DescribeEndpointsResponseReceivedHandler& handler,
        const std::shared_ptr< const Aws::Client::AsyncCallerContext >&
            context = nullptr) const;

    /**
     * <p> Query is a synchronous operation that enables you to execute a query.
     * Query will timeout after 60 seconds. You must update the default timeout
     * in the SDK to support a timeout of 60 seconds. The result set will be
     * truncated to 1MB. Service quotas apply. For more information, see Quotas
     * in the Database Developer Guide.
     */
    virtual QueryOutcome Query(const QueryRequest& request) const;

    /**
     * <p> Query is a synchronous operation that enables you to execute a query.
     * Query will timeout after 60 seconds. You must update the default timeout
     * in the SDK to support a timeout of 60 seconds. The result set will be
     * truncated to 1MB. Service quotas apply. For more information, see Quotas
     * in the Database Developer Guide. 
     *
     * returns a future to the operation so that it can be executed in parallel
     * to other requests.
     */
    virtual QueryOutcomeCallable QueryCallable(
        const QueryRequest& request) const;

    /**
     * <p> Query is a synchronous operation that enables you to execute a query.
     * Query will timeout after 60 seconds. You must update the default timeout
     * in the SDK to support a timeout of 60 seconds. The result set will be
     * truncated to 1MB. Service quotas apply. For more information, see Quotas
     * in the Database Developer Guide.
     *
     * Queues the request into a thread executor and triggers associated
     * callback when operation has finished.
     */
    virtual void QueryAsync(
        const QueryRequest& request,
        const QueryResponseReceivedHandler& handler,
        const std::shared_ptr< const Aws::Client::AsyncCallerContext >&
            context = nullptr) const;

    void OverrideEndpoint(const std::string& endpoint);

   private:
    void init(const Aws::Client::ClientConfiguration& clientConfiguration);
    void LoadDatabaseQuerySpecificConfig(
        const Aws::Client::ClientConfiguration& clientConfiguration);
    void CancelQueryAsyncHelper(
        const CancelQueryRequest& request,
        const CancelQueryResponseReceivedHandler& handler,
        const std::shared_ptr< const Aws::Client::AsyncCallerContext >& context)
        const;
    void DescribeEndpointsAsyncHelper(
        const DescribeEndpointsRequest& request,
        const DescribeEndpointsResponseReceivedHandler& handler,
        const std::shared_ptr< const Aws::Client::AsyncCallerContext >& context)
        const;
    void QueryAsyncHelper(
        const QueryRequest& request,
        const QueryResponseReceivedHandler& handler,
        const std::shared_ptr< const Aws::Client::AsyncCallerContext >& context)
        const;

    std::string m_uri;
    mutable Aws::Utils::ConcurrentCache< std::string, std::string >
        m_endpointsCache;
    bool m_enableEndpointDiscovery;
    std::string m_configScheme;
    std::shared_ptr< Aws::Utils::Threading::Executor > m_executor;
};