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

#include "saml_credentials_provider.h"

// clang-format off
#include "mylog.h"
#include "odbc.h"
#include <aws/sts/STSClient.h>
#include <aws/sts/model/AssumeRoleWithSAMLRequest.h>
// clang-format on

SAMLCredentialsProvider::SAMLCredentialsProvider(const authentication_options& auth)
    : m_auth(auth) {
}

SAMLCredentialsProvider::~SAMLCredentialsProvider() {
}

Aws::Auth::AWSCredentials SAMLCredentialsProvider::GetAWSCredentials() {
    LogMsg(LOG_DEBUG, "Constructing an AssumeRoleWithSAMLRequest.");
    Aws::STS::Model::AssumeRoleWithSAMLRequest saml_request;
    saml_request = saml_request.WithRoleArn(m_auth.role_arn.c_str())
                       .WithSAMLAssertion(GetSAMLAssertion())
                       .WithPrincipalArn(m_auth.idp_arn.c_str());

    LogMsg(LOG_DEBUG, "Fetching the AWS credentials with the SAML assertion.");
    Aws::STS::STSClient sts_client;
    auto outcome = sts_client.AssumeRoleWithSAML(saml_request);
    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError().GetMessage();
        LogMsg(LOG_ERROR, err.c_str());
        throw std::runtime_error("AssumeRoleWithSAMLRequest failed. " + err);
    }
    auto creds = outcome.GetResult().GetCredentials();
    Aws::Auth::AWSCredentials credentials(creds.GetAccessKeyId(),
                                          creds.GetSecretAccessKey(),
                                          creds.GetSessionToken());
    return credentials;
}

void SAMLCredentialsProvider::LogMsg(LogLevel level, const char* msg) {
#if WIN32
#pragma warning(push)
#pragma warning(disable : 4551)
#endif  // WIN32
    // cppcheck outputs an erroneous missing argument error which breaks build.
    // Disable for this function call
    MYLOG(level, "%s", msg);
#if WIN32
#pragma warning(pop)
#endif  // WIN32
}
