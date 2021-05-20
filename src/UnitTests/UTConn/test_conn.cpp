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
#include "pch.h"
#include "unit_test_helper.h"
#include "ts_communication.h"
#include "okta_credentials_provider.h"
// clang-format on

TEST(TestConnectionOptions, Good) {
    runtime_options options;
    options.auth.uid = "UID";
    options.auth.pwd = "PWD";
    options.auth.region = "Region";
    options.auth.auth_type = AUTHTYPE_IAM;
    TSCommunication conn;    
    EXPECT_NO_THROW(conn.Validate(options));
    EXPECT_TRUE(conn.Validate(options));
}

TEST(TestConnectionOptions, UID_is_empty) {
    runtime_options options;
    options.auth.uid = "";
    options.auth.pwd = "PWD";
    options.auth.region = "Region";
    options.auth.auth_type = AUTHTYPE_IAM;
    TSCommunication conn;
    EXPECT_THROW(conn.Validate(options), std::invalid_argument);
}

TEST(TestConnectionOptions, PWD_is_empty) {
    runtime_options options;
    options.auth.uid = "UID";
    options.auth.pwd = "";
    options.auth.region = "Region";
    options.auth.auth_type = AUTHTYPE_IAM;
    TSCommunication conn;
    EXPECT_THROW(conn.Validate(options), std::invalid_argument);
}

TEST(TestConnectionOptions, Region_is_empty) {
    runtime_options options;
    options.auth.uid = "UID";
    options.auth.pwd = "PWD";
    options.auth.region = "";
    options.auth.auth_type = AUTHTYPE_IAM;
    TSCommunication conn;
    EXPECT_THROW(conn.Validate(options), std::invalid_argument);
}

TEST(TestConnectionOptions, Auth_type_is_empty) {
    runtime_options options;
    options.auth.uid = "UID";
    options.auth.pwd = "PWD";
    options.auth.region = "Region";
    options.auth.auth_type = "";
    TSCommunication conn;
    EXPECT_THROW(conn.Validate(options), std::invalid_argument);
}

TEST(TestConnectionOptions, Timeout_is_alpha) {
    runtime_options options;
    options.auth.uid = "UID";
    options.auth.pwd = "PWD";
    options.auth.region = "Region";
    options.auth.auth_type = "";
    options.conn.timeout = "timeout";
    TSCommunication conn;
    EXPECT_THROW(conn.Validate(options), std::invalid_argument);
}

TEST(TestDecodeHex, Single_hex) {
    const std::string hex_encoded = "&#x3d;";
    const std::string expected = "=";
    EXPECT_EQ(expected, OktaCredentialsProvider::DecodeHex(hex_encoded));
}

TEST(TestDecodeHex, Invalid_single_hex) {
    const std::string hex_encoded = "&#x3d";
    EXPECT_EQ(hex_encoded, OktaCredentialsProvider::DecodeHex(hex_encoded));
}

TEST(TestDecodeHex, Double_ampersand) {
    const std::string hex_encoded = "&&#x3d;";
    const std::string expected = "&=";
    EXPECT_EQ(expected, OktaCredentialsProvider::DecodeHex(hex_encoded));
}

TEST(TestDecodeHex, Hex_with_numbers) {
    const std::string hex_encoded = "12345&#x3d;&#x2b;12345";
    const std::string expected = "12345=+12345";
    EXPECT_EQ(expected, OktaCredentialsProvider::DecodeHex(hex_encoded));
}

TEST(TestDecodeHex, Empty) {
    const std::string hex_encoded = "";
    const std::string expected = "";
    EXPECT_EQ(expected, OktaCredentialsProvider::DecodeHex(hex_encoded));
}

TEST(TestDecodeHex, No_hex) {
    const std::string hex_encoded = "12345abc";
    EXPECT_EQ(hex_encoded, OktaCredentialsProvider::DecodeHex(hex_encoded));
}

TEST(TestDecodeHex, All_possible_hex) {
    const std::string hex_encoded = "&#x2b;&#x2f;&#x3d;&#x2c;&#x2d;&#x5f;";
    const std::string expected = "+/=,-_";
    EXPECT_EQ(expected, OktaCredentialsProvider::DecodeHex(hex_encoded));
}

TEST(TestGetUserAgent, Success) {
    TSCommunication conn;
    std::string expected = "ts-odbc." TIMESTREAMDRIVERVERSION " [ut_conn]";
    EXPECT_EQ(expected, conn.GetUserAgent());
}

// TODO: enable gmock and mock the response from timestream
//class TestTSConnConnectDBStart : public testing::Test {
//   protected:
//    void SetUp() override {
//    }
//
//    void TearDown() override {
//        m_conn.Disconnect();
//    }
//
//   public:
//    TSCommunication m_conn;
//};
//
//TEST_F(TestTSConnConnectDBStart, ValidParameters) {
//    ASSERT_NE(false, m_conn.ConnectionOptions(valid_opt_val));
//    EXPECT_EQ(true, m_conn.ConnectDBStart());
//    EXPECT_EQ(CONNECTION_OK, m_conn.Status());
//}
//
//TEST_F(TestTSConnConnectDBStart, InvalidParameters) {
//    ASSERT_TRUE(
//        m_conn.ConnectionOptions(invalid_opt_val));
//    EXPECT_EQ(false, m_conn.ConnectDBStart());
//    EXPECT_EQ(CONNECTION_BAD, m_conn.Status());
//}
//
//TEST_F(TestTSConnConnectDBStart, MissingParameters) {
//    ASSERT_NE(true, m_conn.ConnectionOptions(missing_opt_val));
//    EXPECT_EQ(false, m_conn.ConnectDBStart());
//    EXPECT_EQ(CONNECTION_BAD, m_conn.Status());
//}
//
//TEST(TestTSConnDropDBConnection, InvalidParameters) {
//    TSCommunication conn;
//    ASSERT_EQ(CONNECTION_BAD, conn.Status());
//    ASSERT_TRUE(
//        conn.ConnectionOptions(invalid_opt_val));
//    ASSERT_NE(true, conn.ConnectDBStart());
//    ASSERT_EQ(CONNECTION_BAD, conn.Status());
//    conn.Disconnect();
//    EXPECT_EQ(CONNECTION_BAD, conn.Status());
//}
//
//TEST(TestTSConnDropDBConnection, MissingParameters) {
//    TSCommunication conn;
//    ASSERT_EQ(CONNECTION_BAD, conn.Status());
//    ASSERT_NE(true, conn.ConnectionOptions(missing_opt_val));
//    ASSERT_NE(true, conn.ConnectDBStart());
//    ASSERT_EQ(CONNECTION_BAD, conn.Status());
//    conn.Disconnect();
//    EXPECT_EQ(CONNECTION_BAD, conn.Status());
//}
//
//TEST(TestTSConnDropDBConnection, ValidParameters) {
//    TSCommunication conn;
//    ASSERT_NE(false,
//              conn.ConnectionOptions(valid_opt_val));
//    ASSERT_NE(false, conn.ConnectDBStart());
//    ASSERT_EQ(CONNECTION_OK, conn.Status());
//    conn.Disconnect();
//    EXPECT_EQ(CONNECTION_BAD, conn.Status());
//}

int main(int argc, char** argv) {
    testing::internal::CaptureStdout();
    ::testing::InitGoogleTest(&argc, argv);

    int failures = RUN_ALL_TESTS();

    std::string output = testing::internal::GetCapturedStdout();
    std::cout << output << std::endl;
    std::cout << (failures ? "Not all tests passed." : "All tests passed")
              << std::endl;
    WriteFileIfSpecified(argv, argv + argc, "-fout", output);

    return failures;
}
