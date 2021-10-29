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

project(odbcdriver)

# Source files for odbcdriver
set(C_SOURCE_FILES	bind.c
					columninfo.c
					connection.c
					convert.c
					descriptor.c
					dlg_specific.c
					drvconn.c
					options.c
					environ.c
					execute.c
					info.c
					loadlib.c
					misc.c
					multibyte.c
					mylog.c
					tuple.c
					parse.c
					results.c
					statement.c
					odbcapi30.c
					qresult.c
					odbcapi30w.c
					odbcapi.c
					odbcapiw.c
					win_unicode.c
					api30.c
					types.c
					odbc.c)

set(CPP_SOURCE_FILES	communication.cpp
						connection.cpp
						driver_connect.cpp
						helper.cpp
						info.cpp
						parse_result.cpp
						odbc_statement.cpp
						utility.cpp
						odbc_communication.cpp
						prefetch_queue.cpp)
						
if(WIN32)
set(C_SOURCE_FILES ${C_SOURCE_FILES} dlg_wingui.c setup.c)
endif()

set(C_HEADER_FILES	bind.h
					catfunc.h
					columninfo.h
					convert.h
					descriptor.h
					dlg_specific.h
					drvconn.h
					environ.h
					loadlib.h
					misc.h
					multibyte.h
					mylog.h	
					resource.h
					statement.h
					tuple.h
					unicode_support.h
					qresult.h
					win_setup.h)
	
set(CPP_HEADER_FILES	communication.h
						apifunc.h
						connection.h
						driver_connect.h
						helper.h
						info.h
						odbc.h
						parse_result.h
						odbc_statement.h
						types.h
						utility.h
						odbc_communication.h
						prefetch_queue.h
						version.h)

# Generate dll (SHARED)
if(WIN32)
set(RESOURCE_FILES ts_odbc.rc)
add_library(odbcdriver SHARED ${C_SOURCE_FILES} ${C_HEADER_FILES} ${CPP_SOURCE_FILES} ${CPP_HEADER_FILES} ${RESOURCE_FILES} ${AWSSDK_LIB_DIR})
else()
add_library(odbcdriver SHARED ${C_SOURCE_FILES} ${C_HEADER_FILES} ${CPP_SOURCE_FILES} ${CPP_HEADER_FILES})
endif()

include_directories(
						${CMAKE_CURRENT_SOURCE_DIR}
						${ENLIST_SRC} 
						${AWSSDK_INCLUDE_DIR}
					)

# Platform specific library dependencies
if(WIN32)
	# Windows specifiec
	target_link_libraries(odbcdriver wsock32 ws2_32 winmm user32 gdi32 legacy_stdio_definitions aws-cpp-sdk-core aws-cpp-sdk-sts aws-cpp-sdk-timestream-query kernel32 advapi32 secur32 XOleHlp Wldap32 crypt32 Normaliz odbccp32 odbc32)
	target_link_libraries(odbcdriver debug msvcrtd)
	target_link_libraries(odbcdriver optimized msvcrt)
elseif(APPLE)
	# Apple specific
	target_link_libraries(odbcdriver iodbc iodbcinst aws-cpp-sdk-core aws-cpp-sdk-sts aws-cpp-sdk-timestream-query)
elseif(UNIX)
	# Unix specific
	include_directories(/usr/src/linux-headers-5.0.0-27/include)
	target_link_libraries(odbcdriver aws-cpp-sdk-core aws-cpp-sdk-sts aws-cpp-sdk-timestream-query odbc odbcinst)
	if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9))
		if (Boost_FOUND)
			include_directories(${Boost_INCLUDE_DIRS})
		else()
			message(FATAL_ERROR, "Boost is required for GCC 4.8 and lower, but not found.")
		endif()
	endif()
endif()
