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

#include "performance_test_runner.h"

/******************************************
 * Main
 *
 * 0 command line arguments
 * - default mode:
 * - dsn = documentdb-perf-test
 * - test plan file = Performance_Test_Plan.csv
 * - test results file = Performance_Test_Results.csv
 *
 * 1 command line argument
 * - argv[1] = data source name (dsn)
 *
 * 2 command line arguments
 * - argv[1] - test plan input csv file
 * - argv[2] - test results output csv file
 *
 * 3 command line arguments
 * - argv[1] - test plan input csv file
 * - argv[2] - test results output csv file
 * - argv[3] = data source name (dsn)
 *****************************************/

int main(int argc, char* argv[]) {
  // first argument is program name

  // Initialize variables
  std::string data_source_name, test_plan_file, test_results_file;

  if (argc == 1) {
    // default mode
    data_source_name = dsn_default;
    test_plan_file = input_file;
    test_results_file = output_file;
  } else {
    std::cout << "You have entered " << (argc-1) << " arguments:\n";
    for (int i = 1; i < argc; i++) {
      std::cout << argv[i] << "\n";
    }

    if (argc == 2) {
      // dsn passed in
      data_source_name = argv[1];
      test_plan_file = input_file;
      test_results_file = output_file;
    } else if (argc == 3) {
      // input and output file passed in
      data_source_name = dsn_default;
      test_plan_file = argv[1];
      test_results_file = argv[2];
    } else if (argc == 4) {
      // input file, output file and dsn passed in
      test_plan_file = argv[1];
      test_results_file = argv[2];
      data_source_name = argv[3];
    } else {
      std::cerr << "ERROR: invalid number of command line arguments\n";
    }
  }

  try {
    // Initialize performance test runner
    performance::PerformanceTestRunner performanceTest(
        test_plan_file, test_results_file, data_source_name);

    // Connect to test data source name
    if (!performanceTest.setupConnection()) {
      throw std::runtime_error(performanceTest.getErrorMsg());
    }

    // Read test plan csv file
    if (!performanceTest.readPerformanceTestPlan()) {
      throw std::runtime_error(performanceTest.getErrorMsg());
    }

    // Run performance test
    if (!performanceTest.runPerformanceTestPlan()) {
      throw std::runtime_error(performanceTest.getErrorMsg());
    }
  } catch (const std::runtime_error err) {
    std::cerr << err.what() << std::endl;
  }
}