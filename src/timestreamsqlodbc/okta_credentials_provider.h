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

#ifndef OKTA_CREDENTIALS_PROVIDER
#define OKTA_CREDENTIALS_PROVIDER

#include <string>

#include "saml_credentials_provider.h"

/**
 * Okta SAML authentication to provide AWS credentials.
 */
class OktaCredentialsProvider : public SAMLCredentialsProvider {
   public:
    /**
     * Constructor
     * @param auth Authentication options.
     */
    explicit OktaCredentialsProvider(const authentication_options& auth);
    /**
     * Helper function to decode hex code in a base64 encoded string to ASCII
     * characters. e.g. "&#x3d;" -> "="
     * @param input base64 encoded string with hex code that need to be decoded
     * to ascii characters.
     * @return a base64 encoded string without any hex code.
     */
    static std::string DecodeHex(const std::string& input);

   protected:
    /**
     * Fetches the SAML Assertion from Okta.
     * @return The SAML assertion needed to get temporary credentials from AWS.
     */
    virtual Aws::String GetSAMLAssertion() override;

   private:
    /*
     * length for one single hex code appeared in Okta saml response.
     * e.g. "&#x3d;"
     */
    static const size_t SINGLE_HEX_CODE_LENGTH = 6;

    /**
     * Fetches an access token from Okta Oauth2 endpoint using the provided
     * properties.
     * @return Session token from Okta.
     */
    std::string GetOktaSessionToken() const;
};

#endif
