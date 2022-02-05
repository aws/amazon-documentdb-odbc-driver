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

find_package(ODBC REQUIRED)
include_directories(SYSTEM ${ODBC_INCLUDE_DIRS})
include_directories(lib include src)

# Add source to this project's executable.
add_executable (performance "src/performance.cpp"
							"lib/csv_cell.h"
							"lib/csv_error.h"
							"lib/csv_parser.h"
							"src/performance_odbc_helper.cpp"
							"include/performance_odbc_helper.h" 
							"src/performance_test_runner.cpp"
							"include/performance_test_runner.h")

target_compile_definitions(performance PUBLIC _UNICODE UNICODE)
target_link_libraries(performance ${ODBC_LIBRARY})
set_target_properties(performance PROPERTIES CXX_STANDARD 17)

add_custom_command(
	TARGET performance POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/Performance_Test_Plan.csv
			${CMAKE_SOURCE_DIR}/../build/odbc/cmake/tests/performance/Performance_Test_Plan.csv)
