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

#ifndef AAD_CREDENTIALS_PROVIDER
#define AAD_CREDENTIALS_PROVIDER

#include <aws/core/utils/base64/Base64.h>

#include <string>

#include "saml_credentials_provider.h"

/**
 * Azure Active Directory SAML authentication to provide AWS credentials.
 */
class AADCredentialsProvider : public SAMLCredentialsProvider {
   public:
    /**
     * Constructor
     * @param auth Authentication options.
     */
    explicit AADCredentialsProvider(const authentication_options& auth);

   protected:
    /**
     * Fetches the SAML Assertion from Azure AD.
     * @return The SAML assertion needed to get temporary credentials from AWS.
     */
    virtual Aws::String GetSAMLAssertion() override;

   private:
    /**
     * Fetches an access token from Azure AD Oauth2 endpoint using the provided
     * properties.
     * @return Azure AD access token.
     */
    std::string GetAADAccessToken() const;
};

#endif
