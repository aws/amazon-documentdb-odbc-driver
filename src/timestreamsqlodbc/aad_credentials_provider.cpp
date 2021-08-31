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

#include "aad_credentials_provider.h"

#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/HttpClient.h>
#include <aws/core/http/HttpClientFactory.h>
#include <aws/core/http/standard/StandardHttpRequest.h>
#include <aws/core/http/standard/StandardHttpResponse.h>
#include <aws/core/utils/Array.h>
#include <aws/core/utils/json/JsonSerializer.h>

#include <sstream>

#include "mylog.h"

namespace {
// Base64URL encoding table
const char BASE64_ENCODING_TABLE_URL[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// Base64URL instace to encode or decode
const Aws::Utils::Base64::Base64 BASE64_URL =
    Aws::Utils::Base64::Base64(BASE64_ENCODING_TABLE_URL);

const std::string COLON = "%3A";
}  // namespace

AADCredentialsProvider::AADCredentialsProvider(const authentication_options& auth)
    : SAMLCredentialsProvider(auth) {
}

std::string AADCredentialsProvider::GetAADAccessToken() const {
    auto http_client =
        Aws::Http::CreateHttpClient(Aws::Client::ClientConfiguration());
    Aws::String access_token_endpoint = "https://login.microsoftonline.com/"
                                        + m_auth.aad_tenant + "/oauth2/token";
    auto req = Aws::Http::CreateHttpRequest(
        access_token_endpoint, Aws::Http::HttpMethod::HTTP_POST,
        Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);
    req->SetHeaderValue(Aws::Http::ACCEPT_HEADER, "application/json");
    req->SetHeaderValue(Aws::Http::CONTENT_TYPE_HEADER,
                        "application/x-www-form-urlencoded");

    auto aws_ss = Aws::MakeShared< Aws::StringStream >("");
    *aws_ss << "grant_type=password&requested_token_type=urn" << COLON << "ietf"
            << COLON << "params" << COLON << "oauth" << COLON << "token-type"
            << COLON
            << "saml2&username=" << Aws::Http::URI::URLEncodePath(m_auth.uid)
            << "&password=" << Aws::Http::URI::URLEncodePath(m_auth.pwd)
            << "&client_secret="
            << Aws::Http::URI::URLEncodePath(m_auth.aad_client_secret)
            << "&client_id="
            << Aws::Http::URI::URLEncodePath(m_auth.aad_application_id)
            << "&resource="
            << Aws::Http::URI::URLEncodePath(m_auth.aad_application_id);

    req->AddContentBody(aws_ss);
    req->SetContentLength(std::to_string(aws_ss.get()->str().size()));

    std::shared_ptr< Aws::Http::HttpResponse > res;
    std::string access_token;

    res = http_client->MakeRequest(req);
    if (res->GetResponseCode() != Aws::Http::HttpResponseCode::OK) {
        std::string req_err =
            "Request to Azure Active Directory for access token failed.";
        if (res->HasClientError()) {
            LogMsg(LOG_ERROR, res->GetClientErrorMessage().c_str());
            req_err += " Client error: '" + res->GetClientErrorMessage() + "'.";
        }
        throw std::runtime_error(req_err);
    }

    auto res_json = Aws::Utils::Json::JsonValue(res->GetResponseBody());
    if (!res_json.WasParseSuccessful()) {
        throw std::runtime_error("Error parsing response body. "
                                 + res_json.GetErrorMessage());
    }

    auto json_view = res_json.View();
    if (json_view.ValueExists("access_token")) {
        access_token = json_view.GetString("access_token");
    } else {
        throw std::runtime_error(
            "Unable to extract the access token from the Azure AD response "
            "body.");
    }

    return access_token;
}

Aws::String AADCredentialsProvider::GetSAMLAssertion() {
    std::string access_token = GetAADAccessToken();
    // Microsoft Azure AD doesn't send tail padding to us,
    // the size of the access_token may not be a multiple of 4.
    // While AWS::Utils::Base64 is expecting the size of the encoded is a multiple of 4.
    // we need to pad ourself for the AWS::Utils::Base64 decoder
    auto mod = access_token.size() % 4;
    switch (mod) {
        case 1:
            access_token += "===";
            break;
        case 2:
            access_token += "==";
            break;
        case 3:
            access_token += "=";
            break;
    }
    // Base64URL decode
    auto decode_buffer = BASE64_URL.Decode(access_token);
    auto size = decode_buffer.GetLength();

    std::string decoded(
        reinterpret_cast< char const* >(decode_buffer.GetUnderlyingData()),
        size);

    std::string assertion =
        "<samlp:Response "
        "xmlns:samlp=\"urn:oasis:names:tc:SAML:2.0:protocol\"><samlp:Status><"
        "samlp:StatusCode "
        "Value=\"urn:oasis:names:tc:SAML:2.0:status:Success\"/></samlp:Status>"
        + decoded + "</samlp:Response>";

    Aws::Utils::ByteBuffer encode_buffer = Aws::Utils::ByteBuffer(
        reinterpret_cast< unsigned char const* >(assertion.c_str()),
        assertion.size());
    return BASE64_URL.Encode(encode_buffer);
}
