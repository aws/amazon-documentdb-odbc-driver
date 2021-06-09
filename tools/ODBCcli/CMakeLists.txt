cmake_minimum_required(VERSION 3.17)
project(ODBCcli)

set(CMAKE_CXX_STANDARD 17)

if(APPLE)
    add_compile_definitions(WITH_IODBC)
    include_directories(/usr/local/include)
elseif(UNIX)
    add_compile_definitions(WITH_UNIXODBC)
endif()

add_executable(ODBCcli main.cpp client.cpp client.h)

if(WIN32)
    target_link_libraries(ODBCcli odbc32)
elseif(APPLE)
    target_link_libraries(ODBCcli iodbc)
elseif(UNIX)
    target_link_libraries(ODBCcli unixodbc)
endif()
