#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

project(ignite-odbc-tests)

set(TARGET ${PROJECT_NAME})

if (WIN32)
    set(Boost_USE_STATIC_LIBS ON)
endif()

find_package(Boost 1.53 REQUIRED COMPONENTS unit_test_framework chrono thread system regex)

find_package(ODBC REQUIRED)

include_directories(SYSTEM ${ODBC_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${JNI_INCLUDE_DIRS})
include_directories(include ../odbc/include)
if (WIN32)
    include_directories(../odbc/os/win/include)
else ()
    # TODO: Ensure MacOS is portable. https://bitquill.atlassian.net/browse/AD-525
    include_directories(../odbc/os/linux/include)
endif()

set(SOURCES 
         src/connection_test.cpp
         src/dummy_test.cpp
         src/java_test.cpp
         src/odbc_test_suite.cpp
         src/test_utils.cpp
         ../odbc/src/app/application_data_buffer.cpp
         ../odbc/src/app/parameter.cpp
         ../odbc/src/app/parameter_set.cpp
         ../odbc/src/column.cpp
         ../odbc/src/common/big_integer.cpp
         ../odbc/src/common/bits.cpp
         ../odbc/src/common/concurrent.cpp
         ../odbc/src/common/decimal.cpp
         ../odbc/src/common/utils.cpp
         ../odbc/src/common_types.cpp
         ../odbc/src/config/configuration.cpp
         ../odbc/src/config/config_tools.cpp
         ../odbc/src/config/connection_info.cpp
         ../odbc/src/config/connection_string_parser.cpp
         ../odbc/src/connection.cpp
         ../odbc/src/cursor.cpp
         ../odbc/src/diagnostic/diagnosable_adapter.cpp
         ../odbc/src/diagnostic/diagnostic_record_storage.cpp
         ../odbc/src/diagnostic/diagnostic_record.cpp
         ../odbc/src/dsn_config.cpp
         ../odbc/src/environment.cpp
         ../odbc/src/ignite_error.cpp
         ../odbc/src/jni/java.cpp
         ../odbc/src/log.cpp
         ../odbc/src/message.cpp
         ../odbc/src/meta/column_meta.cpp
         ../odbc/src/meta/table_meta.cpp
         ../odbc/src/nested_tx_mode.cpp
         ../odbc/src/protocol_version.cpp
         ../odbc/src/query/batch_query.cpp
         ../odbc/src/query/column_metadata_query.cpp
         ../odbc/src/query/data_query.cpp
         ../odbc/src/query/foreign_keys_query.cpp
         ../odbc/src/query/primary_keys_query.cpp
         ../odbc/src/query/special_columns_query.cpp
         ../odbc/src/query/streaming_query.cpp
         ../odbc/src/query/type_info_query.cpp
         ../odbc/src/query/table_metadata_query.cpp
         ../odbc/src/sql/sql_lexer.cpp
         ../odbc/src/sql/sql_parser.cpp
         ../odbc/src/sql/sql_set_streaming_command.cpp
         ../odbc/src/sql/sql_utils.cpp
         ../odbc/src/result_page.cpp
         ../odbc/src/row.cpp
         ../odbc/src/ssl_mode.cpp
         ../odbc/src/statement.cpp
         ../odbc/src/streaming/streaming_batch.cpp
         ../odbc/src/streaming/streaming_context.cpp
         ../odbc/src/type_traits.cpp
         ../odbc/src/utility.cpp
        )

if (WIN32)
    list(APPEND SOURCES
        ../odbc/os/win/src/common/concurrent_os.cpp
        ../odbc/os/win/src/common/platform_utils.cpp
        ../odbc/src/jni/os/win/utils.cpp
        ../odbc/os/win/src/system_dsn.cpp
        ../odbc/os/win/src/system/ui/custom_window.cpp
        ../odbc/os/win/src/system/ui/dsn_configuration_window.cpp
        ../odbc/os/win/src/system/ui/window.cpp
    )
else()
    # TODO: Ensure MacOS is portable. https://bitquill.atlassian.net/browse/AD-525
    list(APPEND SOURCES
        ../odbc/os/linux/src/common/concurrent_os.cpp
        ../odbc/os/linux/src/common/platform_utils.cpp
        ../odbc/src/jni/os/linux/utils.cpp
    )
endif()

add_executable(${TARGET} ${SOURCES})

target_link_libraries(${TARGET} ${ODBC_LIBRARIES})
target_link_libraries(${TARGET} ${ODBC_LIBRARY})
target_link_libraries(${TARGET} ${Boost_LIBRARIES} ignite)
target_code_coverage(${TARGET}  PUBLIC AUTO ALL)

if (WIN32)
    remove_definitions(-DUNICODE=1)
    add_definitions(-DTARGET_MODULE_FULL_NAME="$<TARGET_FILE_NAME:${TARGET}>")
    if (MSVC_VERSION GREATER_EQUAL 1900)
        target_link_libraries(${TARGET} legacy_stdio_definitions odbccp32 shlwapi)
    endif()

else()
    add_definitions(-DBOOST_TEST_DYN_LINK)
    target_link_libraries(${TARGET} odbcinst)
endif()

set(TEST_TARGET IgniteOdbcTest)

add_test(NAME ${TEST_TARGET} COMMAND ${TARGET} --catch_system_errors=no --log_level=all)

set_tests_properties(${TEST_TARGET} PROPERTIES ENVIRONMENT IGNITE_NATIVE_TEST_ODBC_CONFIG_PATH=${PROJECT_SOURCE_DIR}/config)
