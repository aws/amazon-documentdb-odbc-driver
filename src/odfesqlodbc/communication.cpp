/*
 * Copyright <2019> Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "communication.h"
// clang-format off
#include "es_odbc.h"
#include "mylog.h"
// clang-format on

Communication::Communication()
    : m_status(ConnStatusType::CONNECTION_BAD),
      m_valid_connection_options(false),
      m_client_encoding(m_supported_client_encodings[0]) {
    LogMsg(ES_ALL, "Initializing Aws API.");
    Aws::InitAPI(m_options);
}

Communication::~Communication() {
    LogMsg(ES_ALL, "Shutting down Aws API.");
    Aws::ShutdownAPI(m_options);
}
      
std::string Communication::GetErrorMessage() {
    if (m_error_details) {
        m_error_details->details = std::regex_replace(
            m_error_details->details, std::regex("\\n"), "\\\\n");
        return GetErrorPrefix() + m_error_details->reason + ": "
               + m_error_details->details;
    } else {
        return GetErrorPrefix() + "No error details available; check the driver logs.";
    }
}

ConnErrorType Communication::GetErrorType() {
    return m_error_type;
}

void Communication::SetErrorDetails(std::string reason, std::string message, ConnErrorType error_type) {
	// Prepare document and validate schema
	auto error_details = std::make_shared< ErrorDetails >();
	error_details->reason = reason;
	error_details->details = message;
	error_details->source_type = "Dummy type";
	error_details->type = error_type;
	m_error_details = error_details;
}

void Communication::SetErrorDetails(ErrorDetails details) {
	// Prepare document and validate schema
	auto error_details = std::make_shared< ErrorDetails >(details);
	m_error_details = error_details;
}

ConnStatusType Communication::GetConnectionStatus() {
    return m_status;
}

bool Communication::ConnectDBStart() {
    LogMsg(ES_ALL, "Starting DB connection.");
    m_status = ConnStatusType::CONNECTION_BAD;
    if (!m_valid_connection_options) {
        // TODO: get error message from CheckConnectionOptions
        m_error_message =
            "Invalid connection options, unable to connect to DB.";
        SetErrorDetails("Invalid connection options", m_error_message,
                        ConnErrorType::CONN_ERROR_COMM_LINK_FAILURE);
        LogMsg(ES_ERROR, m_error_message.c_str());
        DropDBConnection();
        return false;
    }

    m_status = ConnStatusType::CONNECTION_NEEDED;
    if (!EstablishConnection()) {
        m_error_message = "Failed to establish connection to DB.";
        SetErrorDetails("Connection error", m_error_message,
                        ConnErrorType::CONN_ERROR_COMM_LINK_FAILURE);
        LogMsg(ES_ERROR, m_error_message.c_str());
        DropDBConnection();
        return false;
    }

    LogMsg(ES_DEBUG, "Connection established.");
    m_status = ConnStatusType::CONNECTION_OK;
    return true;
}

void Communication::LogMsg(ESLogLevel level, const char* msg) {
#if WIN32
#pragma warning(push)
#pragma warning(disable : 4551)
#endif  // WIN32
    // cppcheck outputs an erroneous missing argument error which breaks build.
    // Disable for this function call
    MYLOG(level, "%s\n", msg);
#if WIN32
#pragma warning(pop)
#endif  // WIN32
}

std::string Communication::GetClientEncoding() {
    return m_client_encoding;
}

bool Communication::SetClientEncoding(std::string& encoding) {
    if (std::find(m_supported_client_encodings.begin(),
                  m_supported_client_encodings.end(), encoding)
        != m_supported_client_encodings.end()) {
        m_client_encoding = encoding;
        return true;
    }
    LogMsg(ES_ERROR,
           std::string("Failed to find encoding " + encoding).c_str());
    return false;
}
