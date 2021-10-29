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

project(tests)

# Source, headers, and include dirs
file(GLOB SOURCE_FILES "*.cpp")
include_directories(	${UT_HELPER}
						${IT_HELPER}
						${ODBC_SRC})

# Generate executable
add_executable(tests ${SOURCE_FILES})

# Library dependencies
target_code_coverage(tests PUBLIC AUTO ALL)
target_link_libraries(tests odbcdriver itodbc_helper ut_helper gtest_main)
target_compile_definitions(tests PUBLIC _UNICODE UNICODE)
