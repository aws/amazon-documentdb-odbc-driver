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
         src/configuration_test.cpp
         src/connection_test.cpp
         src/java_test.cpp
         src/jni_test.cpp
         src/meta_queries_test.cpp
         src/odbc_test_suite.cpp
         src/test_utils.cpp
         src/dummy_test.cpp
        )


add_executable(${TARGET} ${SOURCES})

target_link_libraries(${TARGET} ${ODBC_LIBRARIES})
target_link_libraries(${TARGET} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ignite ignite-odbc mongo::mongocxx_shared)
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
