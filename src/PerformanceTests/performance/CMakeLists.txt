# CMakeList.txt : CMake project for tests, include source and define
# project specific logic here.
#
#
#   Copyright <2021> Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License").
#   You may not use this file except in compliance with the License.
#   A copy of the License is located at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#   or in the "license" file accompanying this file. This file is distributed
#   on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
#   express or implied. See the License for the specific language governing
#   permissions and limitations under the License.
#
cmake_minimum_required (VERSION 3.8)

project(performance)

# Add source to this project's executable.
add_executable (performance "performance.cpp"
							"csv_cell.h"
							"csv_error.h"
							"csv_parser.h"
							"it_odbc_helper.cpp"
							"it_odbc_helper.h" 
							"performance_test_runner.cpp"
							"performance_test_runner.h")

target_compile_definitions(performance PUBLIC _UNICODE UNICODE)

add_custom_command(
	TARGET performance POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/Performance_Test_Plan.csv
			${CMAKE_SOURCE_DIR}/../build/odbc/cmake/PerformanceTests/performance/Performance_Test_Plan.csv)

			add_custom_command(
	TARGET performance POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/Performance_Test_Results.csv
			${CMAKE_SOURCE_DIR}/../build//odbc/cmake/PerformanceTests/performance/Performance_Test_Results.csv)