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
#include <stdexcept>
#include "communication.h"
// clang-format off
#include "es_odbc.h"
#include "mylog.h"

// clang-format on

Communication::Communication()
    : m_status(ConnStatusType::CONNECTION_BAD),
      m_client_encoding(m_supported_client_encodings[0]) {
    LogMsg(ES_ALL, "Initializing AWS API.");
    Aws::InitAPI(m_sdk_options);
}

Communication::~Communication() {
    LogMsg(ES_ALL, "Shutting down AWS API.");
    Aws::ShutdownAPI(m_sdk_options);
}

bool Communication::Setup(const runtime_options& options) {
    if (Validate(options)) {
        if (Connect(options)) {
            m_status = ConnStatusType::CONNECTION_OK;
            return true;
        } else {
            m_status = ConnStatusType::CONNECTION_NEEDED;
            return false;
        }
    }
    return false;
}

ConnStatusType Communication::GetStatus() {
    return m_status;
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

bool Communication::SetClientEncoding(const std::string& encoding) {
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
