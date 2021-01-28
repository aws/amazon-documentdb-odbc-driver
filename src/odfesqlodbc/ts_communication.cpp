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
#include <aws/core/auth/AWSCredentials.h>
#include <aws/timestream-query/model/QueryRequest.h>

// clang-format on

//static const std::string ctype = "application/json";
//static const std::string SQL_ENDPOINT_FORMAT_JDBC =
//    "/_opendistro/_sql?format=jdbc";
//static const std::string SQL_ENDPOINT_CLOSE_CURSOR = "/_opendistro/_sql/close";
//static const std::string PLUGIN_ENDPOINT_FORMAT_JSON =
//    "/_cat/plugins?format=json";
//static const std::string OPENDISTRO_SQL_PLUGIN_NAME = "opendistro_sql";
//static const std::string ALLOCATION_TAG = "IAM_AUTH";
//static const std::string SERVICE_NAME = "es";
//static const std::string ESODBC_PROFILE_NAME = "elasticsearchodbc";
//static const std::string ERROR_MSG_PREFIX =
//    "[Open Distro For Elasticsearch][SQL ODBC Driver][SQL Plugin] ";
//static const std::string JSON_SCHEMA =
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
//    "\"required\": [\"schema\", \"total\", \"datarows\", \"size\", \"status\"]"
//    "}";
//static const std::string CURSOR_JSON_SCHEMA =
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
//static const std::string ERROR_RESPONSE_SCHEMA = R"EOF(
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

void TSCommunication::AwsHttpResponseToString(
    std::shared_ptr< Aws::Http::HttpResponse > /*response*/, std::string& /*output*/) {
    /*
    // This function has some unconventional stream operations because we need
    // performance over readability here. Equivalent code done in conventional
    // ways (using stringstream operators) takes ~30x longer than this code
    // below and bottlenecks our query performance

    // Get streambuffer from response and set position to start
    std::streambuf* stream_buffer = response->GetResponseBody().rdbuf();
    stream_buffer->pubseekpos(0);

    // Get size of streambuffer and reserver that much space in the output
    size_t avail = static_cast< size_t >(stream_buffer->in_avail());
    std::vector< char > buf(avail, '\0');
    output.clear();
    output.reserve(avail);

    // Directly copy memory from buffer into our string buffer
    stream_buffer->sgetn(buf.data(), avail);
    output.assign(buf.data(), avail);
    */
}

// void TSCommunication::PrepareCursorResult(ESResult& es_result) {
    /*
    // Prepare document and validate result
    try {
        LogMsg(DRV_DEBUG, "Parsing result JSON with cursor.");
        es_result.es_result_doc.parse(es_result.result_json,
                                      CURSOR_JSON_SCHEMA);
    } catch (const rabbit::parse_error& e) {
        // The exception rabbit gives is quite useless - providing the json
        // will aid debugging for users
        std::string str = "Exception obtained '" + std::string(e.what())
                          + "' when parsing json string '"
                          + es_result.result_json + "'.";
        throw std::runtime_error(str.c_str());
    }
    */
// }

//std::shared_ptr< ErrorDetails > TSCommunication::ParseErrorResponse(
//    ESResult& es_result) {
    /*
    // Prepare document and validate schema
    try {
        LogMsg(DRV_DEBUG, "Parsing error response (with schema validation)");
        es_result.es_result_doc.parse(es_result.result_json,
                                      ERROR_RESPONSE_SCHEMA);

        auto error_details = std::make_shared< ErrorDetails >();
        error_details->reason =
            es_result.es_result_doc["error"]["reason"].as_string();
        error_details->details =
            es_result.es_result_doc["error"]["details"].as_string();
        error_details->source_type =
            es_result.es_result_doc["error"]["type"].as_string();
        return error_details;
    } catch (const rabbit::parse_error& e) {
        // The exception rabbit gives is quite useless - providing the json
        // will aid debugging for users
        std::string str = "Exception obtained '" + std::string(e.what())
                          + "' when parsing json string '"
                          + es_result.result_json + "'.";
        throw std::runtime_error(str.c_str());
    }
    */
// }

// void TSCommunication::GetJsonSchema(ESResult& es_result) {
    /*
    // Prepare document and validate schema
    try {
        LogMsg(DRV_DEBUG, "Parsing result JSON with schema.");
        es_result.es_result_doc.parse(es_result.result_json, JSON_SCHEMA);
    } catch (const rabbit::parse_error& e) {
        // The exception rabbit gives is quite useless - providing the json
        // will aid debugging for users
        std::string str = "Exception obtained '" + std::string(e.what())
                          + "' when parsing json string '"
                          + es_result.result_json + "'.";
        throw std::runtime_error(str.c_str());
    }
    */
// }


bool TSCommunication::Validate(const runtime_options& options) {
    if (options.auth.uid.empty()) {
        throw std::invalid_argument("UID / AccessKeyId cannot be empty.");
    }
    if (options.auth.pwd.empty()) {
        throw std::invalid_argument("PWD / SecretAccessKey cannot be empty.");
    }
    if (options.auth.region.empty() && options.auth.end_point.empty()) {
        throw std::invalid_argument("Both region and end point cannot be empty.");
    }
    if (options.auth.auth_type != AUTHTYPE_IAM &&
        options.auth.auth_type != AUTHTYPE_AAD &&
        options.auth.auth_type != AUTHTYPE_OKTA) {
        throw std::invalid_argument("Unknown authentication type: \"" + options.auth.auth_type + "\".");
    }
    if (!options.conn.timeout.empty()) {
        std::stol(options.conn.timeout);
    }
    LogMsg(DRV_DEBUG, "Required connection options are valid.");
    return true;
}

bool TSCommunication::Connect(const runtime_options& options) {
    Aws::Client::ClientConfiguration config;
    if (!options.auth.end_point.empty()) {
        config.endpointOverride = options.auth.end_point;
        LogMsg(DRV_ALL, "Disconnecting timestream connection.");
    } else {
        config.enableEndpointDiscovery = true;
        config.region = options.auth.region;
    }
    config.verifySSL = options.crypt.verify_server;
    long request_timeout = static_cast< long >(DEFAULT_REQUEST_TIMEOUT);
    request_timeout = std::stol(options.conn.timeout);
    config.requestTimeoutMs = request_timeout;
    long connection_timeout = static_cast< long >(DEFAULT_CONNECTION_TIMEOUT);
    connection_timeout = std::stol(options.conn.connection_timeout);
    config.connectTimeoutMs = connection_timeout;
    long max_connections = static_cast< long >(DEFAULT_MAX_CONNECTIONS);
    max_connections = std::stol(options.conn.max_connections);
    config.maxConnections = max_connections;

    if (options.auth.auth_type == AUTHTYPE_IAM) {
        Aws::Auth::AWSCredentials credentials(options.auth.uid,
                                              options.auth.pwd, options.auth.session_token);
        m_client =
            std::make_unique< Aws::TimestreamQuery::TimestreamQueryClient >(
                credentials, config);
    } else if (options.auth.auth_type == AUTHTYPE_OKTA) {
    } else {
        throw std::runtime_error("Unknown auth type.");
    }
    
    if (m_client == nullptr) {
        throw std::runtime_error("Unable to create TimestreamQueryClient.");
    }

    Aws::TimestreamQuery::Model::QueryRequest req;
    req.SetQueryString("select 1");
    auto outcome = m_client->Query(req);
    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError().GetMessage();
        LogMsg(DRV_ERROR, err.c_str());
        Disconnect();
        throw std::runtime_error("Failed to establish connection: " + err);
    }
    return true;
}

void TSCommunication::Disconnect() {
    LogMsg(DRV_ALL, "Disconnecting timestream connection.");
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
    const std::string& /*fetch_size*/, const std::string& /*cursor*/) {
    return nullptr;
}

std::vector< std::string > TSCommunication::GetColumnsWithSelectQuery(
    const std::string& /*table_name*/) {
    //std::vector< std::string > list_of_column;
    //if (table_name.empty()) {
    //    m_error_type = ConnErrorType::CONN_ERROR_INVALID_NULL_PTR;
    //    m_error_message = "Query is NULL";
    //    LogMsg(DRV_ERROR, m_error_message.c_str());
    //    return list_of_column;
    //}

    //// Prepare query
    //std::string query = "SELECT * FROM " + table_name + " LIMIT 0";
    //std::string msg = "Attempting to execute a query \"" + query + "\"";
    //LogMsg(DRV_DEBUG, msg.c_str());

    //// Issue request
    //std::shared_ptr< Aws::Http::HttpResponse > response =
    //    IssueRequest(SQL_ENDPOINT_FORMAT_JDBC, Aws::Http::HttpMethod::HTTP_POST,
    //                 ctype, query);

    //// Validate response
    //if (response == nullptr) {
    //    m_error_message =
    //        "Failed to receive response from query. "
    //        "Received NULL response.";
    //    SetErrorDetails("HTTP client error", m_error_message,
    //                    ConnErrorType::CONN_ERROR_COMM_LINK_FAILURE);
    //    LogMsg(DRV_ERROR, m_error_message.c_str());
    //    return list_of_column;
    //}

    //// Convert body from Aws IOStream to string
    //std::unique_ptr< ESResult > result = std::make_unique< ESResult >();
    //AwsHttpResponseToString(response, result->result_json);

    //// If response was not valid, set error
    //if (response->GetResponseCode() != Aws::Http::HttpResponseCode::OK) {
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
    //    LogMsg(DRV_ERROR, m_error_message.c_str());
    //    return list_of_column;
    //}

    //GetJsonSchema(*result);

    //rabbit::array schema_array = result->es_result_doc["schema"];
    //for (rabbit::array::iterator it = schema_array.begin();
    //     it != schema_array.end(); ++it) {
    //    std::string column_name = it->at("name").as_string();
    //    list_of_column.push_back(column_name);
    //}

    //return list_of_column;
    return std::vector< std::string >{};
}

int TSCommunication::ExecDirect(const char* query, const char* fetch_size_) {
    //m_error_details.reset();
    //if (!query) {
    //    m_error_message = "Query is NULL";
    //    SetErrorDetails("Execution error", m_error_message,
    //                    ConnErrorType::CONN_ERROR_INVALID_NULL_PTR);
    //    LogMsg(DRV_ERROR, m_error_message.c_str());
    //    return -1;
    //} else if (!m_client) {
    //    m_error_message = "Unable to connect. Please try connecting again.";
    //    SetErrorDetails("Execution error", m_error_message,
    //                    ConnErrorType::CONN_ERROR_COMM_LINK_FAILURE);
    //    LogMsg(DRV_ERROR, m_error_message.c_str());
    //    return -1;
    //}

    // Prepare statement
    std::string statement(query);
    std::string size(fetch_size_);
    std::string msg = "Attempting to execute a query \"" + statement + "\"";
    LogMsg(DRV_DEBUG, msg.c_str());

    // Issue request
    Aws::TimestreamQuery::Model::QueryRequest request;
    request.SetQueryString(query);
    Aws::TimestreamQuery::Model::QueryOutcome outcome = m_client->Query(request);
    if (outcome.IsSuccess()) {
    } else {
        LogMsg(DRV_ERROR, outcome.GetError().GetMessage().c_str());
        return -1;
    }

    // Add to result queue and return
    // TODO Need to redesign ESResultQueue, see https://github.com/Bit-Quill/timestream-odbc/pull/9#discussion_r558804240
    std::unique_ptr< ESResult > result = std::make_unique< ESResult >();
    result->ts_result = outcome.GetResult();

    const std::string cursor = result->cursor;
    while (!m_result_queue.push(QUEUE_TIMEOUT, result.get())) {
        if (ConnStatusType::CONNECTION_OK == m_status) {
            return -1;
        }
    }
    result.release();

    if (!cursor.empty()) {
        // If the response has a cursor, this thread will retrieve more result
        // pages asynchronously.
        std::thread([&, cursor]() { SendCursorQueries(cursor); }).detach();
    }

    return 0;
}

void TSCommunication::SendCursorQueries(const std::string& /*cursor*/) {
    //if (cursor.empty()) {
    //    return;
    //}
    //m_is_retrieving = true;

    //try {
    //    while (!cursor.empty() && m_is_retrieving) {
    //        std::shared_ptr< Aws::Http::HttpResponse > response = IssueRequest(
    //            SQL_ENDPOINT_FORMAT_JDBC, Aws::Http::HttpMethod::HTTP_POST,
    //            ctype, "", "", cursor);
    //        if (response == nullptr) {
    //            m_error_message =
    //                "Failed to receive response from cursor. "
    //                "Received NULL response.";
    //            SetErrorDetails("Cursor error", m_error_message,
    //                            ConnErrorType::CONN_ERROR_QUERY_SYNTAX);
    //            LogMsg(DRV_ERROR, m_error_message.c_str());
    //            return;
    //        }

    //        std::unique_ptr< ESResult > result = std::make_unique< ESResult >();
    //        AwsHttpResponseToString(response, result->result_json);
    //        PrepareCursorResult(*result);

    //        if (result->es_result_doc.has("cursor")) {
    //            cursor = result->es_result_doc["cursor"].as_string();
    //            result->cursor = result->es_result_doc["cursor"].as_string();
    //        } else {
    //            SendCloseCursorRequest(cursor);
    //            cursor.clear();
    //        }

    //        while (m_is_retrieving
    //               && !m_result_queue.push(QUEUE_TIMEOUT, result.get())) {
    //        }

    //        // Don't release when attempting to push to the queue as it may take
    //        // multiple tries.
    //        result.release();
    //    }
    //} catch (std::runtime_error& e) {
    //    m_error_message =
    //        "Received runtime exception: " + std::string(e.what());
    //    SetErrorDetails("Cursor error", m_error_message,
    //                    ConnErrorType::CONN_ERROR_QUERY_SYNTAX);
    //    LogMsg(DRV_ERROR, m_error_message.c_str());
    //}

    //if (!m_is_retrieving) {
    //    m_result_queue.clear();
    //} else {
    //    m_is_retrieving = false;
    //}
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
        LogMsg(DRV_ERROR, m_error_message.c_str());
    }*/
}

// TODO Need to map Timestream::QueryResult to ESResult
//void TSCommunication::ConstructESResult(ESResult& result) {
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
