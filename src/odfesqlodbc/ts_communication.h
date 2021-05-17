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
     * @param stmt StatementClass *
     * @param query const char*
     * @return int
     */
    virtual int ExecDirect(StatementClass* stmt, const char* query) override;
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
     * Fetches an access token from Azure AD Oauth2 endpoint using the provided properties.
     * @param auth const authentication_options&
     * @return std::string
     */ 
    std::string GetAccessToken(const authentication_options& auth);

    /**
     * Constructs the SAML Assertion with the access token.
     * @param auth const authentication_options&
     * @return Aws::String
     */
    Aws::String GetSAMLAssertion(const authentication_options& auth);
    /**
     * Timestream query client
     */
    std::unique_ptr< Aws::TimestreamQuery::TimestreamQueryClient > m_client;
};

#endif
