cmake_minimum_required(VERSION 3.17)
project(ODBCcli)

set(CMAKE_CXX_STANDARD 14)

add_executable(ODBCcli main.cpp)
target_link_libraries(ODBCcli odbc32)
