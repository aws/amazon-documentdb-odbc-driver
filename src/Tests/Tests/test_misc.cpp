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
#define TEST_STR "The Quick Brown Fox Jumps Over The Lazy Dog"
#define TEST_STR_TRUNCATED "The Quick Brown Fox Jumps Over The Lazy Do"
#define TEST_STR_LOWER "the quick brown fox jumps over the lazy dog"
#define TEST_STR_LOWER_TRUNCATED "the quick brown fox jumps over the lazy do"

void test_copy(const char* src, size_t len, char* exp_dst, size_t exp_bytes_copied) {
    char dst[BUFFER_LEN];
    size_t bytes_copied = strncpy_null(dst, src, len);
    if (src != nullptr) {
        EXPECT_EQ(exp_dst, std::string(dst));
    }
    EXPECT_EQ(exp_bytes_copied, bytes_copied);
}

void test_copy_lower_case(const char* src, size_t len, char* exp_dst, size_t exp_bytes_copied) {
    char dst[BUFFER_LEN];
    size_t bytes_copied = strncpy_lower_null(dst, src, len);
    if (src != nullptr) {
        EXPECT_EQ(exp_dst, std::string(dst));
    }
    EXPECT_EQ(exp_bytes_copied, bytes_copied);
}

TEST(TestMisc, strncpy_null) {
    test_copy(nullptr, BUFFER_LEN, "", 0);
    test_copy("", BUFFER_LEN, "", 0);
    test_copy(TEST_STR, strlen(TEST_STR), TEST_STR_TRUNCATED, strlen(TEST_STR_TRUNCATED));
    test_copy(TEST_STR, strlen(TEST_STR) + 1, TEST_STR, strlen(TEST_STR));
    test_copy(TEST_STR, strlen(TEST_STR) + 2, TEST_STR, strlen(TEST_STR));
    test_copy(TEST_STR, (size_t)999999, TEST_STR, strlen(TEST_STR));
}

TEST(TestMisc, strncpy_lower_null) {
    test_copy_lower_case(nullptr, BUFFER_LEN, "", 0);
    test_copy_lower_case("", BUFFER_LEN, "", 0);
    test_copy_lower_case(TEST_STR, strlen(TEST_STR), TEST_STR_LOWER_TRUNCATED,
                         strlen(TEST_STR_LOWER_TRUNCATED));
    test_copy_lower_case(TEST_STR, strlen(TEST_STR) + 1, TEST_STR_LOWER,
                         strlen(TEST_STR_LOWER));
    test_copy_lower_case(TEST_STR, strlen(TEST_STR) + 2, TEST_STR_LOWER,
                         strlen(TEST_STR_LOWER));
    test_copy_lower_case(TEST_STR, (size_t)999999, TEST_STR_LOWER,
                         strlen(TEST_STR_LOWER));
}
