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

project(odfesqlodbc)

# Source files for odfesqlodbc
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
					es_api30.c
					es_types.c
					es_odbc.c)

set(CPP_SOURCE_FILES	communication.cpp
						es_connection.cpp
						es_driver_connect.cpp
						es_helper.cpp
						es_info.cpp
						es_parse_result.cpp
						es_semaphore.cpp
						es_statement.cpp
						es_utility.cpp
						ts_communication.cpp
						ts_prefetch_queue.cpp
						saml_credentials_provider.cpp
						aad_credentials_provider.cpp
						okta_credentials_provider.cpp)
						
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
						es_apifunc.h
						es_connection.h
						es_driver_connect.h
						es_helper.h
						es_info.h
						es_odbc.h
						es_parse_result.h
						es_semaphore.h
						es_statement.h
						es_types.h
						es_utility.h
						ts_communication.h
						ts_prefetch_queue.h
						saml_credentials_provider.h
						aad_credentials_provider.h
						okta_credentials_provider.h
						version.h)

# Generate dll (SHARED)
if(WIN32)
set(RESOURCE_FILES ts_odbc.rc)
add_library(odfesqlodbc SHARED ${C_SOURCE_FILES} ${C_HEADER_FILES} ${CPP_SOURCE_FILES} ${CPP_HEADER_FILES} ${RESOURCE_FILES} ${AWSSDK_LIB_DIR})
else()
add_library(odfesqlodbc SHARED ${C_SOURCE_FILES} ${C_HEADER_FILES} ${CPP_SOURCE_FILES} ${CPP_HEADER_FILES})
endif()

include_directories(
						${LIBRARY_DIRECTORY}/../src
						${CMAKE_CURRENT_SOURCE_DIR}
						${ODFEENLIST_SRC} 
						${RABBIT_SRC}
						${RAPIDJSON_SRC}
						${AWSSDK_INCLUDE_DIR}
					)

# Platform specific library dependencies
if(WIN32)
	# Windows specifiec
	target_link_libraries(odfesqlodbc wsock32 ws2_32 winmm user32 gdi32 legacy_stdio_definitions aws-cpp-sdk-core aws-cpp-sdk-sts aws-cpp-sdk-timestream-query kernel32 advapi32 secur32 XOleHlp Wldap32 crypt32 Normaliz odbccp32 odbc32)
	target_link_libraries(odfesqlodbc debug msvcrtd)
	target_link_libraries(odfesqlodbc optimized msvcrt)
elseif(APPLE)
	# Apple specific
	target_link_libraries(odfesqlodbc iodbc iodbcinst aws-cpp-sdk-core aws-cpp-sdk-sts aws-cpp-sdk-timestream-query)
elseif(UNIX)
	# Unix specific
	include_directories(/usr/src/linux-headers-5.0.0-27/include)
	target_link_libraries(odfesqlodbc aws-cpp-sdk-core aws-cpp-sdk-sts aws-cpp-sdk-timestream-query odbc odbcinst)
endif()
