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

#include "it_odbc_helper.h"
#include <misc.h>

#define BUFFER_LEN 256

const char* test_str = "The Quick Brown Fox Jumps Over The Lazy Dog";
const size_t test_str_len = strlen(test_str);

void copy(char* dst, const char* src, size_t len) {
    memset(dst, 0, BUFFER_LEN);
    strncpy_null(dst, src, len);
}

void copy_lower_case(char* dst, const char* src, size_t len) {
    memset(dst, 0, BUFFER_LEN);
    strncpy_lower_null(dst, src, len);
}

TEST(TestMisc, strncpy_null) {
    char dst[BUFFER_LEN];

    copy(dst, nullptr, BUFFER_LEN);
    EXPECT_EQ("", std::string(dst));

    copy(dst, "", BUFFER_LEN);
    EXPECT_EQ("", std::string(dst));
    
    copy(dst, test_str, test_str_len);
    EXPECT_EQ("The Quick Brown Fox Jumps Over The Lazy Do", std::string(dst));

    copy(dst, test_str, test_str_len + 1);
    EXPECT_EQ("The Quick Brown Fox Jumps Over The Lazy Dog", std::string(dst));

    copy(dst, test_str, test_str_len + 2);
    EXPECT_EQ("The Quick Brown Fox Jumps Over The Lazy Dog", std::string(dst));
}

TEST(TestMisc, strncpy_lower_null) {
    char dst[BUFFER_LEN];

    copy_lower_case(dst, nullptr, BUFFER_LEN);
    EXPECT_EQ("", std::string(dst));

    copy_lower_case(dst, "", BUFFER_LEN);
    EXPECT_EQ("", std::string(dst));
    
    copy_lower_case(dst, test_str, test_str_len);
    EXPECT_EQ("the quick brown fox jumps over the lazy do", std::string(dst));

    copy_lower_case(dst, test_str, test_str_len + 1);
    EXPECT_EQ("the quick brown fox jumps over the lazy dog", std::string(dst));

    copy_lower_case(dst, test_str, test_str_len + 2);
    EXPECT_EQ("the quick brown fox jumps over the lazy dog", std::string(dst));
}
