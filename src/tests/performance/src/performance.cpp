﻿/*
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

#include "performance_test_runner.h"

/******************************************
 * Main
 *
 * 0 command line arguments
 * - default mode:
 * - dsn = documentdb-perf-test
 * - test plan file =
 *\build\odbc\cmake\tests\performance\Performance_Test_Plan.csv
 * - test results file = Performance_Test_Results.csv
 *
 * 1 command line argument
 * - argv[1] string = data source name (dsn)
 *
 * 2 command line arguments
 * - argv[1] string = test plan input csv file
 * - argv[2] string = test results output csv file
 *
 * 3 command line arguments
 * - argv[1] string = test plan input csv file
 * - argv[2] string = test results output csv file
 * - argv[3] string = data source name (dsn)
 *****************************************/

int main(int argc, char* argv[]) {
#ifdef WIN32
  // Enable CRT for detecting memory leaks
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#ifdef __APPLE__
  // Enable malloc logging for detecting memory leaks.
  system("export MallocStackLogging=1");
#endif

  // Initialize variables
  std::string data_source_name, test_plan_file, test_results_file, user, password;
  int output_mode = 0;  // output time for exec/bind/fetch combined

  // Check command line arguments
  if (argc == 1) {
    // default mode
    data_source_name = kDsnDefault;
    test_plan_file = kInputFile;
    test_results_file = kOutputFile;
  } else {
    std::cout << "You have entered " << (argc - 1) << " arguments:\n";
    for (int i = 1; i < argc; i++) {
      std::cout << argv[i] << "\n";
    }

    if (argc == 2) {
      // dsn passed in
      data_source_name = argv[1];
      test_plan_file = kInputFile;
      test_results_file = kOutputFile;
    } else if (argc == 3) {
      // input and output file passed in
      data_source_name = kDsnDefault;
      test_plan_file = argv[1];
      test_results_file = argv[2];
    } else if (argc == 4) {
      // input file, output file and dsn passed in
      test_plan_file = argv[1];
      test_results_file = argv[2];
      data_source_name = argv[3];
    } else if (argc == 6) {
      // input file, output file, dsn, user and password passed in
      test_plan_file = argv[1];
      test_results_file = argv[2];
      data_source_name = argv[3];
      user = argv[4];
      password = argv[5];
    } else {
      std::cerr << "ERROR: invalid number of command line arguments\n";
    }
  }

  // Run performance test
  try {
    // Initialize performance test runner
    performance::PerformanceTestRunner performanceTest(
        test_plan_file, test_results_file, data_source_name, output_mode, user, password);

    performanceTest.SetupConnection();
    performanceTest.ReadPerformanceTestPlan();
    performanceTest.RunPerformanceTestPlan();
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
  }

#ifdef __APPLE__
  // Disable malloc logging and report memory leaks
  system("unset MallocStackLogging");
  system("leaks performance_results > leaks_performance_results");
#endif
}
