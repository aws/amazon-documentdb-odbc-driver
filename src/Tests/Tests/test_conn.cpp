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

// clang-format off
#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "odbc_communication.h"
#include "pch.h"
#include "unit_test_helper.h"
// clang-format on

TEST(TestConnectionOptions, Good) {
    runtime_options options;
    options.auth.uid = "UID";
    options.auth.pwd = "PWD";
    options.auth.auth_type = AUTHTYPE_DEFAULT;
    DBCommunication conn;    
    EXPECT_NO_THROW(conn.Validate(options));
    EXPECT_TRUE(conn.Validate(options));
}

TEST(TestConnectionOptions, UID_is_empty) {
    runtime_options options;
    options.auth.uid = "";
    options.auth.pwd = "PWD";
    options.auth.auth_type = AUTHTYPE_DEFAULT;
    DBCommunication conn;
    EXPECT_THROW(conn.Validate(options), std::invalid_argument);
}

TEST(TestConnectionOptions, PWD_is_empty) {
    runtime_options options;
    options.auth.uid = "UID";
    options.auth.pwd = "";
    options.auth.auth_type = AUTHTYPE_DEFAULT;
    DBCommunication conn;
    EXPECT_THROW(conn.Validate(options), std::invalid_argument);
}

TEST(TestConnectionOptions, Auth_type_is_empty) {
    runtime_options options;
    options.auth.uid = "UID";
    options.auth.pwd = "PWD";
    options.auth.auth_type = "";
    DBCommunication conn;
    EXPECT_THROW(conn.Validate(options), std::invalid_argument);
}

TEST(TestConnectionOptions, Timeout_is_alpha) {
    runtime_options options;
    options.auth.uid = "UID";
    options.auth.pwd = "PWD";
    options.auth.auth_type = "";
    options.conn.timeout = "timeout";
    DBCommunication conn;
    EXPECT_THROW(conn.Validate(options), std::invalid_argument);
}

TEST(TestGetUserAgent, Success) {
    DBCommunication conn;
    std::string expected = "db-odbc." TIMESTREAMDRIVERVERSION " [tests]";
    EXPECT_EQ(expected, conn.GetUserAgent());
}
