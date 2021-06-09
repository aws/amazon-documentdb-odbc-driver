cmake_minimum_required(VERSION 3.17)
project(ODBCcli)

set(CMAKE_CXX_STANDARD 17)

add_executable(ODBCcli main.cpp client.cpp client.h)
target_link_libraries(ODBCcli odbc32)
