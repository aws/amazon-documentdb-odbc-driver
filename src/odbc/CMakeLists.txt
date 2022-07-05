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

project(documentdb-odbc)

set(TARGET ${PROJECT_NAME})

find_package(ODBC REQUIRED)
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
find_package(Java REQUIRED)
find_package(JNI REQUIRED)
find_package(Boost REQUIRED)
include(UseJava)

if (${CODE_COVERAGE}) 
    add_code_coverage()    
endif()

include_directories(SYSTEM ${ODBC_INCLUDE_DIRS} ${JNI_INCLUDE_DIRS} ${MONGOCXX_INCLUDE_DIRS} ${BSONCXX_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
include_directories(include)


set(SOURCES src/app/application_data_buffer.cpp
        src/app/parameter.cpp
        src/app/parameter_set.cpp
        src/binary/binary_containers.cpp
        src/binary/binary_raw_writer.cpp
        src/binary/binary_writer.cpp
        src/binary/binary_reader.cpp
        src/binary/binary_type.cpp
        src/binary/binary_raw_reader.cpp
	src/cluster/cluster_group.cpp
	src/cluster/cluster_node.cpp
	src/cluster/ignite_cluster.cpp
        src/common_types.cpp
        src/common/big_integer.cpp
        src/common/bits.cpp
        src/common/concurrent.cpp
        src/common/decimal.cpp
        src/ignite_error.cpp
        src/common/utils.cpp
        src/config/config_tools.cpp
        src/config/configuration.cpp
        src/config/connection_info.cpp
        src/config/connection_string_parser.cpp
        src/impl/binary/binary_type_manager.cpp
        src/impl/binary/binary_type_impl.cpp
        src/impl/binary/binary_utils.cpp
        src/impl/binary/binary_reader_impl.cpp
        src/impl/binary/binary_type_handler.cpp
        src/impl/binary/binary_writer_impl.cpp
        src/impl/binary/binary_schema.cpp
        src/impl/binary/binary_type_snapshot.cpp
        src/impl/binary/binary_object_header.cpp
        src/impl/binary/binary_object_impl.cpp
        src/impl/binary/binary_field_meta.cpp
	src/impl/binary/binary_type_updater_impl.cpp
        src/impl/cache/cache_affinity_impl.cpp
        src/impl/cache/cache_impl.cpp
        src/impl/cache/query/query_batch.cpp
        src/impl/cache/query/query_impl.cpp
        src/impl/cache/query/continuous/continuous_query_handle_impl.cpp
        src/impl/cluster/cluster_group_impl.cpp
        src/impl/cluster/cluster_node_impl.cpp
        src/impl/cluster/ignite_cluster_impl.cpp
        src/impl/compute/cancelable_impl.cpp
        src/impl/compute/compute_impl.cpp
        src/impl/interop/interop_input_stream.cpp
        src/impl/interop/interop_memory.cpp
        src/impl/interop/interop_output_stream.cpp
        src/impl/interop/interop_target.cpp
	src/impl/interop/interop_external_memory.cpp
        src/impl/handle_registry.cpp
        src/impl/ignite_binding_impl.cpp
        src/impl/ignite_environment.cpp
        src/impl/ignite_impl.cpp
        src/connection.cpp
        src/driver_instance.cpp
        src/cursor.cpp
        src/diagnostic/diagnosable_adapter.cpp
        src/diagnostic/diagnostic_record.cpp
        src/diagnostic/diagnostic_record_storage.cpp
        src/documentdb_column.cpp
        src/documentdb_cursor.cpp
        src/documentdb_row.cpp
        src/jni/database_metadata.cpp
        src/jni/documentdb_connection.cpp
        src/jni/documentdb_connection_properties.cpp
        src/jni/documentdb_database_metadata.cpp
        src/jni/documentdb_mql_query_context.cpp
        src/jni/documentdb_query_mapping_service.cpp
        src/jni/jdbc_column_metadata.cpp
        src/jni/java.cpp
        src/jni/result_set.cpp
        src/environment.cpp
        src/meta/column_meta.cpp
        src/meta/foreign_key_meta.cpp
        src/meta/primary_key_meta.cpp
        src/meta/table_meta.cpp
        src/odbc.cpp
        src/entry_points.cpp
        src/dsn_config.cpp
        src/query/column_metadata_query.cpp
        src/query/data_query.cpp
        src/query/batch_query.cpp
        src/query/foreign_keys_query.cpp
        src/query/primary_keys_query.cpp
        src/query/table_metadata_query.cpp
        src/query/type_info_query.cpp
        src/query/special_columns_query.cpp
        src/sql/sql_parser.cpp
        src/sql/sql_lexer.cpp
        src/sql/sql_set_streaming_command.cpp
        src/sql/sql_utils.cpp
        src/streaming/streaming_batch.cpp
        src/streaming/streaming_context.cpp
        src/ignite.cpp
        src/ssl_mode.cpp
        src/protocol_version.cpp
        src/result_page.cpp
        src/row.cpp
        src/nested_tx_mode.cpp
        src/message.cpp
        src/column.cpp
        src/statement.cpp
        src/type_traits.cpp
        src/utility.cpp
        src/log.cpp
        src/log_level.cpp
        src/read_preference.cpp
        src/scan_method.cpp
        src/date.cpp
        src/guid.cpp
        src/time.cpp
        src/timestamp.cpp)

if (WIN32)
    set(OS_INCLUDE os/win/include)

    list(APPEND SOURCES 
            os/win/src/system_dsn.cpp
            os/win/src/system/ui/custom_window.cpp
            os/win/src/system/ui/dsn_configuration_window.cpp
            os/win/src/system/ui/window.cpp
            os/win/src/common/concurrent_os.cpp
            os/win/src/common/platform_utils.cpp
	    os/win/src/common/dynamic_load_os.cpp
            src/jni/os/win/utils.cpp
            module.def
    )
else()
    set(OS_INCLUDE os/linux/include)

    list(APPEND SOURCES 
            os/linux/src/common/concurrent_os.cpp
            os/linux/src/common/platform_utils.cpp
	    os/linux/src/common/dynamic_load_os.cpp
            src/jni/os/linux/utils.cpp
    )
endif ()

include_directories(${OS_INCLUDE})

add_library(${TARGET} SHARED ${SOURCES} version.rc)

set_target_properties(${TARGET} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})

target_link_libraries(${TARGET} ${ODBC_LIBRARIES})
target_link_libraries(${TARGET} ${JNI_LIBRARIES})
target_link_libraries(${TARGET} mongo::mongocxx_shared) 

add_definitions(-DUNICODE=1)
if (WIN32)
    if (MSVC)
        # On Windows, min() and max() are defined macro. This causes a colision with MONGOCXX library.
        # See: http://www.suodenjoki.dk/us/archive/2010/min-max.htm
        add_definitions(-DNOMINMAX)
    endif()

    target_link_libraries(${TARGET} odbccp32 shlwapi)

    add_definitions(-DTARGET_MODULE_FULL_NAME="$<TARGET_FILE_NAME:${TARGET}>")

    if (MSVC_VERSION GREATER_EQUAL 1900)
        target_link_libraries(${TARGET} legacy_stdio_definitions)
    endif()
                
    set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "documentdb.odbc")
else()
    target_link_libraries(${TARGET})
endif()

if (WIN32 AND ${WITH_ODBC_MSI})
    find_program(WIX_CANDLE candle)
    if(NOT WIX_CANDLE)
        message(FATAL_ERROR "WIX candle.exe not found! Have you installed WIX Toolset or forgotten to add it to Path?")
    endif()

    find_program(WIX_LIGHT light)
    if(NOT WIX_LIGHT)
        message(FATAL_ERROR "WIX light.exe not found! Have you installed WIX Toolset or forgotten to add it to Path?")
    endif()

    set(WIX_ODBC_LIBRARY_PATH ".\\documentdb.odbc.dll")
    set(WIX_ODBC_BSON1_PATH ".\\bson-1.0.dll")
    set(WIX_ODBC_BSONCXX_PATH ".\\bsoncxx.dll")
    set(WIX_ODBC_MONGOC_PATH ".\\mongoc-1.0.dll")
    set(WIX_ODBC_MONGOCXX_PATH ".\\mongocxx.dll")
    set(WIX_ODBC_ZLIB1_PATH ".\\zlib1.dll")
    set(WIX_ODBC_JDBC_PATH ".\\libs\\documentdb-jdbc-1.2.4-all.jar")

    set(WIX_PROJECT_NAME "Amazon DocumentDB")
    set(WIX_MANUFACTURER "Amazon Web Services")
    set(WIX_COPYRIGHT_COMMENT "Apache, Apache Ignite, the Apache feather and the Apache Ignite logo are trademarks of The Apache Software Foundation.")

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(WIX_BIT_SUFFIX "64-bit")
        set(WIX_PRODUCT_ID "ae863f63-39ce-4f61-9b19-011bc63d89e9")
        set(WIX_UPGRADE_CODE "b23e4250-96ba-4ad7-ada0-64f307d2e1fd")
        set(WIX_COMPONENT_ID_ODBC_LIBRARY "7cb71911-a963-41ba-8dcc-fe5de6ddfaf1")
        set(WIX_COMPONENT_ID_BSON1 "950288ae-f70b-459a-b079-02887797e3d0")
        set(WIX_COMPONENT_ID_BSONCXX "ce855f2b-384d-4bf2-a62c-f921722eef7e")
        set(WIX_COMPONENT_ID_MONGOC "7ee7d64d-2f03-4c8e-9da1-6995deb7c9ac")
        set(WIX_COMPONENT_ID_MONGOCXX "88eb3917-63e0-4e7c-ae6e-04dae66df013")
        set(WIX_COMPONENT_ID_ZLIB1 "2c920532-81fa-4088-a446-47aaa8db83ae")
        set(WIX_COMPONENT_ID_JDBC "70020c86-713b-423d-b782-79f72ec7a388")
        set(WIX_COMPONENT_ID_DOCUMNETDB_HOME_ENV_VAR "281ea3c0-192c-4a3c-9ddf-5bc1cd77a790")
        set(WIX_PROGRAM_FILES_FOLDER_ID "ProgramFiles64Folder")
        set(WIX_COMPONENT_IS_WIN64 "yes")
        set(WIX_PACKAGE_PLATFORM "x64")
        set(WIX_INSTALLER_PREFIX ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/documentdb-odbc-amd64)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(WIX_BIT_SUFFIX "32-bit")
        set(WIX_PRODUCT_ID "60cb757b-69cb-412c-9d06-f22cd96914ed")
        set(WIX_UPGRADE_CODE "1bc0039e-b824-45ff-8a85-45492bf7eb46")
        set(WIX_COMPONENT_ID_ODBC_LIBRARY "46b68e84-8f98-4c00-a8f9-d81a06bba694")
        set(WIX_COMPONENT_ID_BSON1 "c34b6eaa-3f17-4e54-b3dd-ce95f6a95c5b")
        set(WIX_COMPONENT_ID_BSONCXX "f9d57dda-9051-4032-a23d-251ba681e64a")
        set(WIX_COMPONENT_ID_MONGOC "694ecd9b-7d68-4998-bc82-83d82ba2bde6")
        set(WIX_COMPONENT_ID_MONGOCXX "0aa05333-e7ff-4f32-9d89-0b637475425e")
        set(WIX_COMPONENT_ID_ZLIB1 "41f84e00-d899-493b-a0c4-c59b53998538")
        set(WIX_COMPONENT_ID_JDBC "1ff0aa89-8a06-4c21-b0ce-ff9523ac8f10")
        set(WIX_COMPONENT_ID_DOCUMNETDB_HOME_ENV_VAR "04aff702-9fe4-4dc1-9979-daa94aca5a03")
        set(WIX_PROGRAM_FILES_FOLDER_ID "ProgramFilesFolder")
        set(WIX_COMPONENT_IS_WIN64 "no")
        set(WIX_PACKAGE_PLATFORM "x86")
        set(WIX_INSTALLER_PREFIX ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/documentdb-odbc-x86)
    endif()

    set(WIX_PRODUCT_NAME "${WIX_PROJECT_NAME} ODBC ${WIX_BIT_SUFFIX} Driver")

    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/install/ignite-odbc.wxs.in"
        "${WIX_INSTALLER_PREFIX}.wxs"
        @ONLY
    )

    add_custom_command(
        TARGET ${TARGET} POST_BUILD
        COMMAND ${WIX_CANDLE} ${WIX_INSTALLER_PREFIX}.wxs -out ${WIX_INSTALLER_PREFIX}.wxobj
        COMMAND ${WIX_LIGHT} -ext WixUIExtension ${WIX_INSTALLER_PREFIX}.wxobj -out ${WIX_INSTALLER_PREFIX}.msi
    )
endif()

target_include_directories(${TARGET} INTERFACE include)

install(TARGETS ${TARGET} 
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

if (WIN32 AND ${WITH_ODBC_MSI})
   install(FILES ${WIX_INSTALLER_PREFIX}.msi DESTINATION bin)
endif()

if (WIN32)
    if (MSVC_VERSION GREATER_EQUAL 1900)
        target_link_libraries(${TARGET} legacy_stdio_definitions odbccp32 shlwapi)
    endif()
elseif(APPLE)
    target_link_libraries(${TARGET} iodbcinst)
else()
    target_link_libraries(${TARGET} odbcinst)
endif()

set(VERSIONINFO ${CMAKE_PROJECT_VERSION_MAJOR},${CMAKE_PROJECT_VERSION_MINOR},${CMAKE_PROJECT_VERSION_PATCH})
set(VERSIONINFO_STR ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH})
configure_file("version.rc.in" "version.rc" @ONLY)
