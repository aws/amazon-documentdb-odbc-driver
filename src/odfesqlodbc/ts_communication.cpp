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

#include "ts_communication.h"

// odfesqlodbc needs to be included before mylog, otherwise mylog will generate
// compiler warnings
// clang-format off
#include "es_odbc.h"
#include "mylog.h"
#include <memory>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/client/DefaultRetryStrategy.h>
#include <aws/timestream-query/model/QueryRequest.h>

// clang-format on

// static const std::string ctype = "application/json";
// static const std::string SQL_ENDPOINT_FORMAT_JDBC =
//    "/_opendistro/_sql?format=jdbc";
// static const std::string SQL_ENDPOINT_CLOSE_CURSOR =
// "/_opendistro/_sql/close"; static const std::string
// PLUGIN_ENDPOINT_FORMAT_JSON =
//    "/_cat/plugins?format=json";
//static const std::string OPENDISTRO_SQL_PLUGIN_NAME = "opendistro_sql";
//static const std::string ALLOCATION_TAG = "IAM_AUTH";
//static const std::string SERVICE_NAME = "es";
//static const std::string ESODBC_PROFILE_NAME = "elasticsearchodbc";
//static const std::string ERROR_MSG_PREFIX =
//    "[Open Distro For Elasticsearch][SQL ODBC Driver][SQL Plugin] ";
// static const std::string JSON_SCHEMA =
//    "{"  // This was generated from the example elasticsearch data
//    "\"type\": \"object\","
//    "\"properties\": {"
//    "\"schema\": {"
//    "\"type\": \"array\","
//    "\"items\": [{"
//    "\"type\": \"object\","
//    "\"properties\": {"
//    "\"name\": { \"type\": \"string\" },"
//    "\"type\": { \"type\": \"string\" }"
//    "},"
//    "\"required\": [ \"name\", \"type\" ]"
//    "}]"
//    "},"
//    "\"cursor\": { \"type\": \"string\" },"
//    "\"total\": { \"type\": \"integer\" },"
//    "\"datarows\": {"
//    "\"type\": \"array\","
//    "\"items\": {}"
//    "},"
//    "\"size\": { \"type\": \"integer\" },"
//    "\"status\": { \"type\": \"integer\" }"
//    "},"
//    "\"required\": [\"schema\", \"total\", \"datarows\", \"size\",
//    \"status\"]"
//    "}";
// static const std::string CURSOR_JSON_SCHEMA =
//    "{"  // This was generated from the example elasticsearch data
//    "\"type\": \"object\","
//    "\"properties\": {"
//    "\"cursor\": { \"type\": \"string\" },"
//    "\"datarows\": {"
//    "\"type\": \"array\","
//    "\"items\": {}"
//    "},"
//    "\"status\": { \"type\": \"integer\" }"
//    "},"
//    "\"required\":  [\"datarows\"]"
//    "}";
// static const std::string ERROR_RESPONSE_SCHEMA = R"EOF(
//{
//    "type": "object",
//    "properties": {
//        "error": {
//            "type": "object",
//            "properties": {
//                "reason": { "type": "string" },
//                "details": { "type": "string" },
//                "type": { "type": "string" }
//            },
//            "required": [
//                "reason",
//                "details",
//                "type"
//            ]
//        },
//        "status": {
//            "type": "integer"
//        }
//    },
//    "required": [
//        "error",
//        "status"
//    ]
//}
//)EOF";

// void TSCommunication::PrepareCursorResult(TSResult& ts_result) {

// Prepare document and validate result
//try {
//    LogMsg(LOG_DEBUG, "Parsing result JSON with cursor.");
//    ts_result.es_result_doc.parse(ts_result.result_json,
//                                  CURSOR_JSON_SCHEMA);
//} catch (const rabbit::parse_error& e) {
//    // The exception rabbit gives is quite useless - providing the json
//    // will aid debugging for users
//    std::string str = "Exception obtained '" + std::string(e.what())
//                      + "' when parsing json string '"
//                      + ts_result.result_json + "'.";
//    throw std::runtime_error(str.c_str());
//}
//
//// std::shared_ptr< ErrorDetails > TSCommunication::ParseErrorResponse(
////    TSResult& ts_result) {
//// Prepare document and validate schema
//try {
//    LogMsg(ES_DEBUG, "Parsing error response (with schema validation)");
//    ts_result.es_result_doc.parse(ts_result.result_json,
//                                  ERROR_RESPONSE_SCHEMA);
//    auto error_details = std::make_shared< ErrorDetails >();
//    error_details->reason =
//        ts_result.es_result_doc["error"]["reason"].as_string();
//    error_details->details =
//        ts_result.es_result_doc["error"]["details"].as_string();
//    error_details->source_type =
//        ts_result.es_result_doc["error"]["type"].as_string();
//    return error_details;
//} catch (const rabbit::parse_error& e) {
//    // The exception rabbit gives is quite useless - providing the json
//    // will aid debugging for users
//    std::string str = "Exception obtained '" + std::string(e.what())
//                      + "' when parsing json string '"
//                      + ts_result.result_json + "'.";
//    throw std::runtime_error(str.c_str());
//}
//// }
//
//// void TSCommunication::GetJsonSchema(TSResult& ts_result) {
//// Prepare document and validate schema
//try {
//    LogMsg(ES_DEBUG, "Parsing result JSON with schema.");
//    ts_result.es_result_doc.parse(ts_result.result_json, JSON_SCHEMA);
//} catch (const rabbit::parse_error& e) {
//    // The exception rabbit gives is quite useless - providing the json
//    // will aid debugging for users
//    std::string str = "Exception obtained '" + std::string(e.what())
//                      + "' when parsing json string '"
//                      + ts_result.result_json + "'.";
//    throw std::runtime_error(str.c_str());
//}
// }

bool TSCommunication::Validate(const runtime_options& options) {
    if (options.auth.region.empty() && options.auth.end_point_override.empty()) {
        throw std::invalid_argument("Both region and end point cannot be empty.");
    }
    if (options.auth.auth_type != AUTHTYPE_AWS_PROFILE &&
        options.auth.auth_type != AUTHTYPE_IAM &&
        options.auth.auth_type != AUTHTYPE_AAD &&
        options.auth.auth_type != AUTHTYPE_OKTA) {
        throw std::invalid_argument("Unknown authentication type: \"" + options.auth.auth_type + "\".");
    }
    if (options.auth.auth_type != AUTHTYPE_AWS_PROFILE && options.auth.uid.empty()) {
        throw std::invalid_argument("UID / AccessKeyId cannot be empty.");
    }
    if (options.auth.auth_type != AUTHTYPE_AWS_PROFILE && options.auth.pwd.empty()) {
        throw std::invalid_argument("PWD / SecretAccessKey cannot be empty.");
    }
    LogMsg(LOG_DEBUG, "Required connection options are valid.");
    return true;
}

bool TSCommunication::Connect(const runtime_options& options) {
    Aws::Client::ClientConfiguration config;
    if (!options.auth.end_point_override.empty()) {
        config.endpointOverride = options.auth.end_point_override;
    } else {
        config.enableEndpointDiscovery = true;
        config.region = options.auth.region;
    }
    long request_timeout = static_cast< long >(DEFAULT_REQUEST_TIMEOUT);
    if (!options.conn.timeout.empty()) {
        request_timeout = std::stol(options.conn.timeout);
    }
    if (request_timeout >= 0) {
        config.requestTimeoutMs = request_timeout;
    }
    long connection_timeout = static_cast< long >(DEFAULT_CONNECTION_TIMEOUT);
    if (!options.conn.connection_timeout.empty()) {
        connection_timeout = std::stol(options.conn.connection_timeout);
    }
    if (connection_timeout >= 0) {
        config.connectTimeoutMs = connection_timeout;
    }
    int max_connections = static_cast< int >(DEFAULT_MAX_CONNECTIONS);
    if (!options.conn.max_connections.empty()) {
        max_connections = std::stoi(options.conn.max_connections);
    }
    if (max_connections >= 0) {
        config.maxConnections = max_connections;
    }
    if (!options.conn.max_retry_count_client.empty()) {
        long max_retry_count_client = std::stol(options.conn.max_retry_count_client);
        if (max_retry_count_client < 0) {
            throw std::invalid_argument("Max retry count client cannot be negative.");
        }
        config.retryStrategy = std::make_shared< Aws::Client::DefaultRetryStrategy >(max_retry_count_client);
    }

    if (options.auth.auth_type == AUTHTYPE_AWS_PROFILE) {
        m_client =
            std::make_unique< Aws::TimestreamQuery::TimestreamQueryClient >(config);
    } else if (options.auth.auth_type == AUTHTYPE_IAM) {
        Aws::Auth::AWSCredentials credentials(options.auth.uid,
                                              options.auth.pwd, options.auth.session_token);
        m_client =
            std::make_unique< Aws::TimestreamQuery::TimestreamQueryClient >(
                credentials, config);
    } else if (options.auth.auth_type == AUTHTYPE_AAD) {
        // AAD
    } else if (options.auth.auth_type == AUTHTYPE_OKTA) {
        // OKTA
    } else {
        throw std::runtime_error("Unknown auth type: " + options.auth.auth_type);
    }

    if (m_client == nullptr) {
        throw std::runtime_error("Unable to create TimestreamQueryClient.");
    }

    Aws::TimestreamQuery::Model::QueryRequest req;
    req.SetQueryString("select 1");
    auto outcome = m_client->Query(req);
    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError().GetMessage();
        LogMsg(LOG_ERROR, err.c_str());
        Disconnect();
        throw std::runtime_error("Failed to establish connection: " + err);
    }
    LogMsg(LOG_DEBUG, "Connection Established.");
    return true;
}

void TSCommunication::Disconnect() {
    LogMsg(LOG_ALL, "Disconnecting timestream connection.");
    if (m_client) {
        m_client.reset();
    }
    m_status = ConnStatusType::CONNECTION_BAD;
    StopResultRetrieval();
}

std::string TSCommunication::GetVersion() {
    // AWS SDK version
    return "1.7.329";
}

std::string TSCommunication::GetErrorPrefix() {
    return "[Timestream][SQL ODBC Driver] ";
}
std::shared_ptr< Aws::Http::HttpResponse > TSCommunication::IssueRequest(
    const std::string& /*endpoint*/,
    const Aws::Http::HttpMethod /*request_type*/,
    const std::string& /*content_type*/, const std::string& /*query*/,
    const std::string& /*cursor*/) {
    return nullptr;
}

std::vector< std::string > TSCommunication::GetColumnsWithSelectQuery(
    const std::string& /*table_name*/) {
    // std::vector< std::string > list_of_column;
    // if (table_name.empty()) {
    //    m_error_type = ConnErrorType::CONN_ERROR_INVALID_NULL_PTR;
    //    m_error_message = "Query is NULL";
    //    LogMsg(LOG_ERROR, m_error_message.c_str());
    //    return list_of_column;
    //}

    //// Prepare query
    //std::string query = "SELECT * FROM " + table_name + " LIMIT 0";
    //std::string msg = "Attempting to execute a query \"" + query + "\"";
    //LogMsg(LOG_DEBUG, msg.c_str());

    //// Issue request
    // std::shared_ptr< Aws::Http::HttpResponse > response =
    //    IssueRequest(SQL_ENDPOINT_FORMAT_JDBC,
    //    Aws::Http::HttpMethod::HTTP_POST,
    //                 ctype, query);

    //// Validate response
    // if (response == nullptr) {
    //    m_error_message =
    //        "Failed to receive response from query. "
    //        "Received NULL response.";
    //    SetErrorDetails("HTTP client error", m_error_message,
    //                    ConnErrorType::CONN_ERROR_COMM_LINK_FAILURE);
    //    LogMsg(LOG_ERROR, m_error_message.c_str());
    //    return list_of_column;
    //}

    //// Convert body from Aws IOStream to string
    // std::unique_ptr< TSResult > result = std::make_unique< TSResult >();
    // AwsHttpResponseToString(response, result->result_json);

    //// If response was not valid, set error
    // if (response->GetResponseCode() != Aws::Http::HttpResponseCode::OK) {
    //    m_error_type = ConnErrorType::CONN_ERROR_QUERY_SYNTAX;
    //    m_error_message =
    //        "Http response code was not OK. Code received: "
    //        + std::to_string(static_cast< long >(response->GetResponseCode()))
    //        + ".";
    //    if (response->HasClientError())
    //        m_error_message +=
    //            " Client error: '" + response->GetClientErrorMessage() + "'.";
    //    if (!result->result_json.empty()) {
    //        m_error_message +=
    //            " Response error: '" + result->result_json + "'.";
    //    }
    //    SetErrorDetails("Connection error", m_error_message,
    //                    ConnErrorType::CONN_ERROR_COMM_LINK_FAILURE);
    //    LogMsg(LOG_ERROR, m_error_message.c_str());
    //    return list_of_column;
    //}

    // GetJsonSchema(*result);

    // rabbit::array schema_array = result->es_result_doc["schema"];
    // for (rabbit::array::iterator it = schema_array.begin();
    //     it != schema_array.end(); ++it) {
    //    std::string column_name = it->at("name").as_string();
    //    list_of_column.push_back(column_name);
    //}

    // return list_of_column;
    return std::vector< std::string >{};
}

int TSCommunication::ExecDirect(const char* query) {
    //m_error_details.reset();
    //if (!query) {
    //    m_error_message = "Query is NULL";
    //    SetErrorDetails("Execution error", m_error_message,
    //                    ConnErrorType::CONN_ERROR_INVALID_NULL_PTR);
    //    LogMsg(LOG_ERROR, m_error_message.c_str());
    //    return -1;
    //} else if (!m_client) {
    //    m_error_message = "Unable to connect. Please try connecting again.";
    //    SetErrorDetails("Execution error", m_error_message,
    //                    ConnErrorType::CONN_ERROR_COMM_LINK_FAILURE);
    //    LogMsg(LOG_ERROR, m_error_message.c_str());
    //    return -1;
    //}

    // Prepare statement
    std::string statement(query);
    std::string msg = "Attempting to execute a query \"" + statement + "\"";
    LogMsg(LOG_DEBUG, msg.c_str());

    // Issue request
    Aws::TimestreamQuery::Model::QueryRequest request;
    request.SetQueryString(query);
    auto outcome = m_client->Query(request);
    if (!outcome.IsSuccess()) {
        LogMsg(LOG_ERROR, outcome.GetError().GetMessage().c_str());
        return -1;
    }

    std::unique_ptr< TSResult > ts_result = std::make_unique< TSResult >();
    ts_result->sdk_result = outcome.GetResult();
    ts_result->next_token = outcome.GetResult().GetNextToken();
    while (!m_result_queue.push(QUEUE_TIMEOUT, ts_result.get())) {
        if (ConnStatusType::CONNECTION_OK == m_status) {
            return -1;
        }
    }
    ts_result.release();

    if (!outcome.GetResult().GetNextToken().empty()) {
        auto next_token = outcome.GetResult().GetNextToken();
        std::thread([this, request, next_token]() {
            this->SendCursorQueries(request, next_token);
        }).detach();
    }
    return 0;
}

void TSCommunication::SendCursorQueries(
    Aws::TimestreamQuery::Model::QueryRequest request, Aws::String next_token) {
    if (next_token.empty()) {
        LogMsg(LOG_ERROR, "Next token is empty");
        return;
    }
    m_is_retrieving = true;
    try {
        while (!next_token.empty() && m_is_retrieving) {
            LogMsg(LOG_DEBUG,
                   "SendCursorQueries: Start fetching more result sets in the "
                   "background");
            auto outcome = m_client->Query(request.WithNextToken(next_token));
            if (!outcome.IsSuccess()) {
                LogMsg(LOG_ERROR, outcome.GetError().GetMessage().c_str());
                return;
            }

            std::unique_ptr< TSResult > result = std::make_unique< TSResult >();
            result->sdk_result = outcome.GetResult();
            result->next_token = outcome.GetResult().GetNextToken();

            while (m_is_retrieving
                   && !m_result_queue.push(QUEUE_TIMEOUT, result.get())) {
            }

            next_token = result->next_token;

            // Don't release when attempting to push to the queue as it may take
            // multiple tries.
            result.release();
        }
    } catch (std::runtime_error& e) {
        std::string error_message =
            "Received runtime exception: " + std::string(e.what());
        LogMsg(LOG_ERROR, error_message.c_str());
    }
    LogMsg(LOG_DEBUG, "SendCursorQueriesDone fetching more results");

    if (!m_is_retrieving) {
        m_result_queue.clear();
    } else {
        m_is_retrieving = false;
    }
}

void TSCommunication::SendCloseCursorRequest(const std::string& /*cursor*/) {
    /*std::shared_ptr< Aws::Http::HttpResponse > response =
        IssueRequest(SQL_ENDPOINT_CLOSE_CURSOR,
                     Aws::Http::HttpMethod::HTTP_POST, ctype, "", "", cursor);
    if (response == nullptr) {
        m_error_message =
            "Failed to receive response from cursor close request. "
            "Received NULL response.";
        SetErrorDetails("Cursor error", m_error_message,
                        ConnErrorType::CONN_ERROR_QUERY_SYNTAX);
        LogMsg(LOG_ERROR, m_error_message.c_str());
    }*/
}

// TODO Need to map Timestream::QueryResult to TSResult
// void TSCommunication::ConstructTSResult(TSResult& result) {
//    rabbit::array schema_array = result.es_result_doc["schema"];
//    for (rabbit::array::iterator it = schema_array.begin();
//         it != schema_array.end(); ++it) {
//        std::string column_name = it->at("name").as_string();
//
//        ColumnInfo col_info;
//        col_info.field_name = column_name;
//        col_info.type_oid = KEYWORD_TYPE_OID;
//        col_info.type_size = KEYWORD_TYPE_SIZE;
//        col_info.display_size = KEYWORD_DISPLAY_SIZE;
//        col_info.length_of_str = KEYWORD_TYPE_SIZE;
//        col_info.relation_id = 0;
//        col_info.attribute_number = 0;
//
//        result.column_info.push_back(col_info);
//    }
//    if (result.es_result_doc.has("cursor")) {
//        result.cursor = result.es_result_doc["cursor"].as_string();
//    }
//    result.command_type = "SELECT";
//    result.num_fields = (uint16_t)schema_array.size();
// }
