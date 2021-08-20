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

#define BUFFER_LEN 128
#define SMALL_BUFFER_LEN 10
#define TEST_STR "The Quick Brown Fox Jumps Over The Lazy Dog"
#define TEST_STR_TRUNCATED "The Quick"
#define TEST_STR_UPPER "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG"
#define TEST_STR_LOWER "the quick brown fox jumps over the lazy dog"
#define TEST_STR_LOWER_TRUNCATED "the quick"

void test_to_lower_case(char* str, size_t len, char const *exp_str) {
    to_lower_case(str, len);
    if (exp_str == nullptr) {
        EXPECT_EQ(exp_str, str);
    } else {
        EXPECT_EQ(exp_str, std::string(str));
    }
}

void test_copy(const char* src, char const *exp_dst) {
    char dst[BUFFER_LEN];
    strncpy_null(dst, src, sizeof(dst));
    if (exp_dst == nullptr) {
        EXPECT_EQ(exp_dst, src);
    } else {
        EXPECT_EQ(exp_dst, std::string(dst));
    }
}

void test_copy_small_buffer(const char* src, char const *exp_dst) {
    char dst[SMALL_BUFFER_LEN];
    strncpy_null(dst, src, sizeof(dst));
    if (exp_dst == nullptr) {
        EXPECT_EQ(exp_dst, src);
    } else {
        EXPECT_EQ(exp_dst, std::string(dst));
    }
}

void test_copy_lower_case(const char* src, char const *exp_dst) {
    char dst[BUFFER_LEN];
    strncpy_null_lower_case(dst, src, sizeof(dst));
    if (exp_dst == nullptr) {
        EXPECT_EQ(exp_dst, src);
    } else {
        EXPECT_EQ(exp_dst, std::string(dst));
    }
}

void test_copy_lower_case_small_buffer(const char* src, char const *exp_dst) {
    char dst[SMALL_BUFFER_LEN];
    strncpy_null_lower_case(dst, src, sizeof(dst));
    if (exp_dst == nullptr) {
        EXPECT_EQ(exp_dst, src);
    } else {
        EXPECT_EQ(exp_dst, std::string(dst));
    }
}

TEST(TestMisc, to_lower_case) {
    test_to_lower_case(nullptr, 0, nullptr);
    test_to_lower_case((char*)"", 0, "");
    
    char str[] = TEST_STR;
    test_to_lower_case(str, strlen(str), TEST_STR_LOWER);

    char str_upper[] = TEST_STR_UPPER;
    test_to_lower_case(str_upper, strlen(str_upper), TEST_STR_LOWER);

    char str_lower[] = TEST_STR_LOWER;
    test_to_lower_case(str_lower, strlen(str_lower), TEST_STR_LOWER);
}

TEST(TestMisc, strncpy_null) {    
    test_copy(nullptr, nullptr);
    test_copy("", "");
    test_copy(TEST_STR, TEST_STR);
    test_copy_small_buffer(TEST_STR, TEST_STR_TRUNCATED);
}

TEST(TestMisc, strncpy_null_lower_case) {
    test_copy_lower_case(nullptr, nullptr);
    test_copy_lower_case("", "");
    test_copy_lower_case(TEST_STR, TEST_STR_LOWER);
    test_copy_lower_case(TEST_STR_UPPER, TEST_STR_LOWER);
    test_copy_lower_case(TEST_STR_LOWER, TEST_STR_LOWER);
    test_copy_lower_case_small_buffer(TEST_STR, TEST_STR_LOWER_TRUNCATED);
    test_copy_lower_case_small_buffer(TEST_STR_UPPER, TEST_STR_LOWER_TRUNCATED);
    test_copy_lower_case_small_buffer(TEST_STR_LOWER, TEST_STR_LOWER_TRUNCATED);
}
