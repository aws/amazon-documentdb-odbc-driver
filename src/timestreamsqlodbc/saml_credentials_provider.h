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

#ifndef SAML_CREDENTIALS_PROVIDER
#define SAML_CREDENTIALS_PROVIDER

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>

#include <map>
#include <memory>
#include <string>

#include "types.h"

/**
 * SAML credentials provider class to provider AWS credentials.
 */
class SAMLCredentialsProvider {
   public:
    /**
     * Constructor
     * @param auth Authentication options.
     */
    explicit SAMLCredentialsProvider(const authentication_options& auth);

    /**
     * Destructor
     */
    virtual ~SAMLCredentialsProvider();

    /**
     * Creates an AWSCredentials using the credentials fetched through Okta or
     * Azure AD SAML authentication.
     *
     * @return the AWS Credentials using the provided SAML Assertion.
     */
    Aws::Auth::AWSCredentials GetAWSCredentials();

    /**
     * Log messages
     * @param level Log level.
     * @param msg Log message.
     */
    static void LogMsg(LogLevel level, const char* msg);

   protected:
    /**
     * Fetches the SAML Assertion from the identity provider.
     * @return The SAML assertion needed to get temporary credentials from AWS.
     */
    virtual Aws::String GetSAMLAssertion() = 0;

    /**
     * Authentication options for identity providers.
     */
    authentication_options m_auth;
};

#endif
