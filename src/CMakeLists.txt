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

cmake_minimum_required(VERSION 3.16) 

project(global_make_list)

include("${CMAKE_CURRENT_SOURCE_DIR}/modules/code-coverage.cmake")
add_code_coverage_all_targets(EXCLUDE libraries aws-cpp-sdk googletest Tests)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(BITNESS 64)
	set(BITNESS_NAME "x64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(BITNESS 32)
	set(BITNESS_NAME "x86")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build/odbc/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build/odbc/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build/odbc/bin")

set(CMAKE_CXX_STANDARD 17)
# This wasn't being picked up on mac, causes some symbol errors
if(APPLE)
	add_definitions(-DHAVE_STRLCAT)
	option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
else()
	option(BUILD_SHARED_LIBS "Build shared libraries" ON)
endif()
# Set BUILD_WITH_TESTS to OFF before building installer package for size optimization.
option(BUILD_WITH_TESTS "Enable testing" ON)

# Do not under any circumstance reduce or remove the compiler warning flags.
# These are our (best) friends.
if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -fPIC -Werror)
endif()

# Base directories
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(ODBC_SRC "${CMAKE_CURRENT_SOURCE_DIR}/timestreamsqlodbc")
set(ENLIST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/timestreamenlist")
set(TESTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests")
set(INSTALL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/installer")
set(DSN_INSTALLER_SRC "${CMAKE_CURRENT_SOURCE_DIR}/DSNInstaller")

# ODBC Driver version
set(VERSION_MAJOR 0)
set(VERSION_MINOR 5)
set(VERSION_PATCH 0)

add_compile_definitions( TS_ODBC_VERSION="${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
                         # Comma separated version is required for odbc administrator's driver file.
                         TS_ODBC_DRVFILE_VERSION=${VERSION_MAJOR},${VERSION_MINOR},${VERSION_PATCH} )

# Extensions of base directories
set(PERFORMANCE_TESTS "${CMAKE_CURRENT_SOURCE_DIR}/PerformanceTests")
set(UT_HELPER "${TESTS}/UTHelper")
set(IT_HELPER "${TESTS}/ITODBCHelper")

# Without this symbols will be exporting to Unix but not Windows
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

# Set path for AWS SDK
set(aws-cpp-sdk-base "${CMAKE_CURRENT_SOURCE_DIR}/aws-sdk-cpp")
if(UNIX OR APPLE)
	set(aws-cpp-sdk-core_DIR "${aws-cpp-sdk-base}/install/lib/cmake/aws-cpp-sdk-core")
	set(aws-cpp-sdk-sts_DIR "${aws-cpp-sdk-base}/install/lib/cmake/aws-cpp-sdk-sts")
	set(aws-cpp-sdk-timestream-query_DIR "${aws-cpp-sdk-base}/install/lib/cmake/aws-cpp-sdk-timestream-query")
	# AWS SDK dependencies
	set(aws-crt-cpp_DIR "${aws-cpp-sdk-base}/install/lib/aws-crt-cpp/cmake")
	set(aws-c-io_DIR "${aws-cpp-sdk-base}/install/lib/aws-c-io/cmake")
	set(aws-c-http_DIR "${aws-cpp-sdk-base}/install/lib/aws-c-http/cmake")
	set(aws-c-event-stream_DIR "${aws-cpp-sdk-base}/install/lib/aws-c-event-stream/cmake")
	set(aws-c-common_DIR "${aws-cpp-sdk-base}/install/lib/aws-c-common/cmake")
	set(aws-checksums_DIR "${aws-cpp-sdk-base}/install/lib/aws-checksums/cmake")
	set(s2n_DIR "${aws-cpp-sdk-base}/install/lib/s2n/cmake")
	set(aws-c-cal_DIR "${aws-cpp-sdk-base}/install/lib/aws-c-cal/cmake")
	set(aws-c-compression_DIR "${aws-cpp-sdk-base}/install/lib/aws-c-compression/cmake")
	set(aws-c-mqtt_DIR "${aws-cpp-sdk-base}/install/lib/aws-c-mqtt/cmake")
	set(aws-c-auth_DIR "${aws-cpp-sdk-base}/install/lib/aws-c-auth/cmake")
	set(aws-c-s3_DIR "${aws-cpp-sdk-base}/install/lib/aws-c-s3/cmake")
endif()

if (WIN32)
    find_package(AWSSDK REQUIRED core sts timestream-query)
else()
    find_package(AWSSDK REQUIRED COMPONENTS core sts timestream-query PATHS aws-sdk-cpp/install NO_DEFAULT_PATH)
endif()

# General compiler definitions
add_compile_definitions		(	_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
								UNICODE_SUPPORT
								DYNAMIC_LOAD
								_MBCS
								_CRT_SECURE_NO_DEPRECATE
								_USRDLL
								_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
								_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
							)

# Platform specific compiler definitions
if (WIN32 AND BITNESS EQUAL 64)
	# Windows specific
	add_compile_definitions	(	_WINDLL
								_WIN64
								_WINDOWS
								WIN_EXPORT
							)
elseif (WIN32 AND BITNESS EQUAL 32)
	# Windows specific
	add_compile_definitions	(	_WINDLL
								_WIN32
								_WINDOWS
								WIN_EXPORT
							)
elseif(APPLE)
	# macOS specific
	add_compile_definitions	(	WITH_IODBC
								SQLCOLATTRIBUTE_SQLLEN
							)
	# iODBC includes
	include_directories(/usr/local/include)
elseif(UNIX)
	# Unix specific
	set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
	add_compile_definitions	(	WITH_UNIXODBC
								SQLCOLATTRIBUTE_SQLLEN
							)
endif()

if(BUILD_WITH_TESTS)
	# GTest import
	include(gtest/googletest.cmake)
	fetch_googletest(
		${PROJECT_SOURCE_DIR}/gtest
		${PROJECT_BINARY_DIR}/googletest
		)
	enable_testing()
endif()

# Projects to build
add_subdirectory(${ODBC_SRC})
add_subdirectory(${ENLIST_SRC})
add_subdirectory(${INSTALL_SRC})

# Only build & package DSN installer for Mac platforms
if(APPLE)
	add_subdirectory(${DSN_INSTALLER_SRC})
endif()

if(BUILD_WITH_TESTS)
	add_subdirectory(${TESTS})
	add_subdirectory(${PERFORMANCE_TESTS})
endif()
