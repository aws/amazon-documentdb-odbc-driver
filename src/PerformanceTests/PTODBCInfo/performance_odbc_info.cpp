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
#include "ts_communication.h"
#include "unit_test_helper.h"
#include "it_odbc_helper.h"
#include "chrono"
#include <cwctype>
#include "rabbit.hpp"
#include <codecvt>
#include <locale>
// clang-format on

const std::vector< std::string > base_items = {"name", "cluster_name",
                                               "cluster_uuid"};
const std::vector< std::string > version_items = {
    "number",
    "build_flavor",
    "build_type",
    "build_hash",
    "build_date",
    "build_snapshot",
    "lucene_version",
    "minimum_wire_compatibility_version",
    "minimum_index_compatibility_version"};
const std::string sync_start = "%%__PARSE__SYNC__START__%%";
const std::string sync_sep = "%%__SEP__%%";
const std::string sync_end = "%%__PARSE__SYNC__END__%%";

std::string wstring_to_string(const std::wstring& src) {
    return std::wstring_convert< std::codecvt_utf8_utf16< wchar_t >, wchar_t >{}
        .to_bytes(src);
}

runtime_options rt_opts = []() {
    runtime_options temp_opts;
    for (auto it : conn_str_pair) {
        std::wstring tmp = it.first;
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), towlower);
        if (tmp == IT_AUTH)
            temp_opts.auth.auth_type = wstring_to_string(it.second);
        else if (tmp == IT_ACCESSKEYID)
            temp_opts.auth.uid = wstring_to_string(it.second);
        else if (tmp == IT_SECRETACCESSKEY)
            temp_opts.auth.pwd = wstring_to_string(it.second);
        else if (tmp == IT_REGION)
            temp_opts.auth.region = wstring_to_string(it.second);
    }
    return temp_opts;
}();

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
