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

#include "okta_credentials_provider.h"

#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/HttpClient.h>
#include <aws/core/http/HttpClientFactory.h>
#include <aws/core/http/standard/StandardHttpRequest.h>
#include <aws/core/http/standard/StandardHttpResponse.h>
#include <aws/core/utils/json/JsonSerializer.h>

#include <sstream>

#include "mylog.h"

namespace {
// Regex to search for SAMLResponse from Okta response.
const std::regex SAML_RESPONSE_PATTERN =
    std::regex(R"#(<input name="SAMLResponse" type="hidden" value="(.*?)"/>)#");
}  // namespace

OktaCredentialsProvider::OktaCredentialsProvider(const authentication_options& auth)
    : SAMLCredentialsProvider(auth) {
}

std::string OktaCredentialsProvider::GetOktaSessionToken() const {
    auto http_client =
        Aws::Http::CreateHttpClient(Aws::Client::ClientConfiguration());
    Aws::String session_token_endpoint =
        "https://" + m_auth.idp_host + "/api/v1/authn";
    auto req = Aws::Http::CreateHttpRequest(
        session_token_endpoint, Aws::Http::HttpMethod::HTTP_POST,
        Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);
    req->SetHeaderValue(Aws::Http::ACCEPT_HEADER, "application/json");
    req->SetHeaderValue(Aws::Http::CONTENT_TYPE_HEADER, "application/json");

    auto json_body = Aws::Utils::Json::JsonValue();
    json_body.WithString("username", Aws::String(m_auth.uid));
    if (!json_body.WasParseSuccessful()) {
        throw std::runtime_error(
            "Error adding Okta username to json request body. "
            + json_body.GetErrorMessage());
    }
    json_body.WithString("password", Aws::String(m_auth.pwd));
    if (!json_body.WasParseSuccessful()) {
        throw std::runtime_error(
            "Error adding Okta password to json request body. "
            + json_body.GetErrorMessage());
    }
    auto body = json_body.View().WriteReadable();
    auto aws_ss = Aws::MakeShared< Aws::StringStream >("");
    aws_ss->write(body.c_str(), body.size());

    req->AddContentBody(aws_ss);
    req->SetContentLength(std::to_string(body.size()));

    std::shared_ptr< Aws::Http::HttpResponse > res;
    std::string session_token;

    res = http_client->MakeRequest(req);
    if (res->GetResponseCode() != Aws::Http::HttpResponseCode::OK) {
        std::string req_err = "Request to Okta for session token failed.";
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
    if (json_view.ValueExists("sessionToken")) {
        session_token = json_view.GetString("sessionToken");
    } else {
        throw std::runtime_error(
            "Unable to extract the session token field from the Okta response "
            "body.");
    }

    return session_token;
}

std::string OktaCredentialsProvider::DecodeHex(const std::string& hex_encoded) {
    std::string result;
    result.reserve(hex_encoded.size());
    if (hex_encoded.length() < SINGLE_HEX_CODE_LENGTH)
        return hex_encoded;

    for (size_t i = 0; i < hex_encoded.length(); i++) {
        char c;
        // Check if the next character is the start of a hex code that need
        // to be decoded by checking the first 3 characters for the preamble
        // "&#x" and the 6th character for ';'
        if (hex_encoded.substr(i, 3) == "&#x" && hex_encoded[i + 5] == ';') {
            // Extract the 2-digit hex code. e.g. "&#x3d;" -> "3d"
            std::string extracted_hex = hex_encoded.substr(i + 3, 2);
            unsigned int x;
            std::stringstream ss;
            ss << std::hex << extracted_hex;
            ss >> x;
            c = (char)x;
            // Jump over the current hex code.
            i += 5;
        } else {
            c = hex_encoded[i];
        }
        result.push_back(c);
    }

    return result;
}

Aws::String OktaCredentialsProvider::GetSAMLAssertion() {
    std::string session_token = GetOktaSessionToken();
    auto http_client =
        Aws::Http::CreateHttpClient(Aws::Client::ClientConfiguration());

    Aws::String uri = "https://" + m_auth.idp_host + "/app/amazon_aws/"
                      + m_auth.okta_application_id + "/sso/saml";

    auto req = Aws::Http::CreateHttpRequest(
        uri, Aws::Http::HttpMethod::HTTP_GET,
        Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);

    req->AddQueryStringParameter("onetimetoken", session_token);

    std::shared_ptr< Aws::Http::HttpResponse > res;

    res = http_client->MakeRequest(req);
    if (res->GetResponseCode() != Aws::Http::HttpResponseCode::OK) {
        std::string req_err = "Request to Okta for SAML assertion failed.";
        if (res->HasClientError()) {
            LogMsg(LOG_ERROR, res->GetClientErrorMessage().c_str());
            req_err += " Client error: '" + res->GetClientErrorMessage() + "'.";
        }
        throw std::runtime_error(req_err);
    }

    std::string body(std::istreambuf_iterator< char >(res->GetResponseBody()),
                     {});

    std::smatch matches;
    std::string saml_response;
    if (std::regex_search(body, matches, SAML_RESPONSE_PATTERN)) {
        saml_response = DecodeHex(matches.str(1));
    }

    if (saml_response.empty()) {
        throw std::runtime_error(
            "Unable to extract the SAMLResponse field from the Okta response "
            "body.");
    }

    return saml_response;
}
