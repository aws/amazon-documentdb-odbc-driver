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
#include "pch.h"
#include "unit_test_helper.h"
#include "it_odbc_helper.h"
#include "catfunc.h"
// clang-format on

int main(int argc, char** argv) {
    std::cerr << "Start: Line 30" << std::endl;
#ifdef WIN32
    std::cerr << "Line 32" << std::endl;
    // Enable CRT for detecting memory leaks
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
    std::cerr << "Line 35" << std::endl;
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    std::cerr << "Line 37" << std::endl;
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    std::cerr << "Line 39" << std::endl;
#endif
#ifdef __APPLE__
    // Enable malloc logging for detecting memory leaks.
    system("export MallocStackLogging=1");
#endif
    std::cerr << "Line 45" << std::endl;
    testing::internal::CaptureStdout();
    std::cerr << "Line 47" << std::endl;
    ::testing::InitGoogleTest(&argc, argv);

    std::cerr << "Line 50" << std::endl;
    int failures = RUN_ALL_TESTS();

    std::cerr << "Line 53" << std::endl;
    std::string output = testing::internal::GetCapturedStdout();
    std::cerr << "Line 55" << std::endl;
    std::cout << output << std::endl;
    std::cerr << "Line 56" << std::endl;
    std::cout << (failures ? "Not all tests passed." : "All tests passed")
              << std::endl;
    std::cerr << "Line 60" << std::endl;
    WriteFileIfSpecified(argv, argv + argc, "-fout", output);
    std::cerr << "Line 62" << std::endl;

#ifdef __APPLE__
    // Disable malloc logging and report memory leaks
    system("unset MallocStackLogging");
    system("leaks tests > leaks_tests");
#endif
    return failures;
}
