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

#include <aws/timestream-query/TimestreamQueryClient.h>
#include <aws/timestream-query/model/ScalarType.h>
#include <aws/timestream-query/model/Type.h>

#include <memory>
#include <string>

#include "communication.h"
#include "saml_credentials_provider.h"

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
     * @param stmt StatementClass *
     * @param query const char*
     * @return bool
     */
    virtual bool ExecDirect(StatementClass* stmt, const char* query) override;
    /**
     * Cancel query
     * @param stmt StatementClass *
     * @return bool
     */
    virtual bool CancelQuery(StatementClass* stmt) override;
    /**
     * Get driver version
     * @return std::string
     */
    virtual std::string GetVersion() override;

    /**
     * Get error prefix
     * @return std::string
     */
    virtual std::string GetErrorPrefix() override;

    /**
     * For prefetch mechanism
     * During retrieving data from database, caller can call SQLFreeStmt /
     * SQLCloseCursor to stop it
     * @param stmt StatementClass*
     */
    virtual void StopResultRetrieval(StatementClass* stmt) override;

   private:
    /**
     * Create an unique_ptr of TimestreamQueryClient using the given credentials
     * provider and configs
     * @param cp std::unique_ptr<SAMLCredentialsProvider>
     * @param auth const authentication_options&
     * @param config const Aws::Client::ClientConfiguration&
     * @return std::unique_ptr< Aws::TimestreamQuery::TimestreamQueryClient >
     */
    std::unique_ptr< Aws::TimestreamQuery::TimestreamQueryClient >
    CreateQueryClientWithIdp(std::unique_ptr< SAMLCredentialsProvider > cp,
                             const Aws::Client::ClientConfiguration& config);
    /**
     * Timestream query client
     */
    std::unique_ptr< Aws::TimestreamQuery::TimestreamQueryClient > m_client;
};

#endif
