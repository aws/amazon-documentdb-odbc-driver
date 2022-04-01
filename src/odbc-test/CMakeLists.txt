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
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

find_package(ODBC REQUIRED)

include_directories(SYSTEM ${ODBC_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${JNI_INCLUDE_DIRS} ${MONGOCXX_INCLUDE_DIRS} ${BSONCXX_INCLUDE_DIRS})
include_directories(include ../odbc/include)
if (WIN32)
    include_directories(../odbc/os/win/include)
else ()
    # TODO: Ensure MacOS is portable. https://bitquill.atlassian.net/browse/AD-525
    include_directories(../odbc/os/linux/include)
endif()

set(SOURCES 
         src/api_robustness_test.cpp
         src/configuration_test.cpp
         src/connection_test.cpp
         src/java_test.cpp
         src/jni_test.cpp
         src/meta_queries_test.cpp
         src/odbc_test_suite.cpp
         src/queries_test.cpp
         src/test_utils.cpp
         ../odbc/src/app/application_data_buffer.cpp
         ../odbc/src/binary/binary_containers.cpp
         ../odbc/src/binary/binary_raw_writer.cpp
         ../odbc/src/binary/binary_writer.cpp
         ../odbc/src/binary/binary_reader.cpp
         ../odbc/src/binary/binary_type.cpp
         ../odbc/src/binary/binary_raw_reader.cpp
         ../odbc/src/common_types.cpp
         ../odbc/src/common/big_integer.cpp
         ../odbc/src/common/bits.cpp
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
         ../odbc/src/impl/binary/binary_type_manager.cpp
         ../odbc/src/impl/binary/binary_type_impl.cpp
         ../odbc/src/impl/binary/binary_utils.cpp
         ../odbc/src/impl/binary/binary_reader_impl.cpp
         ../odbc/src/impl/binary/binary_type_handler.cpp
         ../odbc/src/impl/binary/binary_writer_impl.cpp
         ../odbc/src/impl/binary/binary_schema.cpp
         ../odbc/src/impl/binary/binary_type_snapshot.cpp
         ../odbc/src/impl/binary/binary_object_header.cpp
         ../odbc/src/impl/binary/binary_object_impl.cpp
         ../odbc/src/impl/binary/binary_field_meta.cpp
         ../odbc/src/impl/interop/interop_memory.cpp
         ../odbc/src/impl/interop/interop_output_stream.cpp
         ../odbc/src/impl/interop/interop_input_stream.cpp
         ../odbc/src/connection.cpp
         ../odbc/src/driver_instance.cpp
         ../odbc/src/cursor.cpp
         ../odbc/src/diagnostic/diagnosable_adapter.cpp
         ../odbc/src/diagnostic/diagnostic_record_storage.cpp
         ../odbc/src/diagnostic/diagnostic_record.cpp
         ../odbc/src/documentdb_column.cpp
         ../odbc/src/documentdb_cursor.cpp
         ../odbc/src/documentdb_row.cpp
         ../odbc/src/dsn_config.cpp
         ../odbc/src/environment.cpp
         ../odbc/src/ignite_error.cpp
         ../odbc/src/jni/database_metadata.cpp
         ../odbc/src/jni/documentdb_connection.cpp
         ../odbc/src/jni/documentdb_mql_query_context.cpp
         ../odbc/src/jni/documentdb_query_mapping_service.cpp
         ../odbc/src/jni/java.cpp
         ../odbc/src/jni/result_set.cpp
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
         ../odbc/src/query/type_info_query.cpp
         ../odbc/src/query/table_metadata_query.cpp
         ../odbc/src/sql/sql_lexer.cpp
         ../odbc/src/sql/sql_parser.cpp
         ../odbc/src/sql/sql_set_streaming_command.cpp
         ../odbc/src/sql/sql_utils.cpp
         ../odbc/src/read_preference.cpp
         ../odbc/src/result_page.cpp
         ../odbc/src/row.cpp
         ../odbc/src/scan_method.cpp
         ../odbc/src/statement.cpp
         ../odbc/src/streaming/streaming_batch.cpp
         ../odbc/src/streaming/streaming_context.cpp
         ../odbc/src/type_traits.cpp
         ../odbc/src/utility.cpp
         ../odbc/src/scan_method.cpp
         ../odbc/src/date.cpp
         ../odbc/src/guid.cpp
         ../odbc/src/time.cpp
         ../odbc/src/timestamp.cpp
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
target_link_libraries(${TARGET} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ignite mongo::mongocxx_shared)
target_code_coverage(${TARGET}  PUBLIC AUTO ALL)

if (WIN32)
    if (MSVC)
        # On Windows, min() and max() are defined macro. This causes a colision with MONGOCXX library.
        # See: http://www.suodenjoki.dk/us/archive/2010/min-max.htm
        add_definitions(-DNOMINMAX)
    endif()
    remove_definitions(-DUNICODE=1)
    add_definitions(-DTARGET_MODULE_FULL_NAME="$<TARGET_FILE_NAME:${TARGET}>")
    if (MSVC_VERSION GREATER_EQUAL 1900)
        target_link_libraries(${TARGET} legacy_stdio_definitions odbccp32 shlwapi)
    endif()
elseif(APPLE)
    add_definitions(-DBOOST_TEST_DYN_LINK)
    target_link_libraries(${TARGET} iodbcinst)
else()
    add_definitions(-DBOOST_TEST_DYN_LINK)
    target_link_libraries(${TARGET} odbcinst)
endif()

set(TEST_TARGET IgniteOdbcTest)

add_test(NAME ${TEST_TARGET} COMMAND ${TARGET} --catch_system_errors=no --log_level=all)

set_tests_properties(${TEST_TARGET} PROPERTIES ENVIRONMENT IGNITE_NATIVE_TEST_ODBC_CONFIG_PATH=${PROJECT_SOURCE_DIR}/config)
