#include "client.h"
#include <cstring>
#include <stdexcept>
#include "sqlext.h"

Client::Client(const char *connection_string) : henv(SQL_NULL_HENV),
                                                hdbc(SQL_NULL_HDBC),
                                                hstmt(SQL_NULL_HSTMT) {
    // Allocate the environment handle
    auto retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (!SQL_SUCCEEDED(retcode)) {
        throw std::runtime_error("Failed at SQLAllocHandle SQL_HANDLE_ENV " + std::to_string(retcode) + ".");
    }
    // Set the version environment attribute
    retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER *) SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(retcode)) {
        throw std::runtime_error("Failed at SQLSetEnvAttr SQL_ATTR_ODBC_VERSION " + std::to_string(retcode) + ".");
    }
    // Allocate the connection handle
    retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (!SQL_SUCCEEDED(retcode)) {
        throw std::runtime_error("Failed at SQLAllocHandle SQL_HANDLE_DBC " + std::to_string(retcode) + ".");
    }
    // Connect
    SQLCHAR strConnIn[BUF_LEN] = {0}, strConnOut[BUF_LEN] = {0};
    SQLSMALLINT lenConnOut;
    strcpy((char *) strConnIn, connection_string);
    retcode = SQLDriverConnect(hdbc, NULL, strConnIn, SQL_NTS, strConnOut, BUF_LEN, &lenConnOut, SQL_DRIVER_COMPLETE);
    if (!SQL_SUCCEEDED(retcode)) {
        throw std::runtime_error("Failed at SQLDriverConnect with code " + std::to_string(retcode) + ".");
    }
    // Allocate the statement handle
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (!SQL_SUCCEEDED(retcode)) {
        throw std::runtime_error("Failed at SQLAllocHandle SQL_HANDLE_STMT " + std::to_string(retcode) + ".");
    }
}

Client::~Client() {
    // Free statement
    if (hstmt != SQL_NULL_HSTMT) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
    // Free connection
    if (hdbc != SQL_NULL_HDBC) {
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    }
    // Free environment
    if (henv != SQL_NULL_HENV) {
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
    }
}

void Client::Query(const char *query) {
    auto retcode = SQLExecDirect(hstmt, (SQLCHAR *) query, (SQLINTEGER) strlen(query));
    if (!SQL_SUCCEEDED(retcode)) {
        throw std::runtime_error("Failed at SQLExecDirect " + std::to_string(retcode) + ".");
    }
}

std::optional<std::vector<std::string>> Client::FetchOne() {
    auto retcode = SQLFetch(hstmt);
    if (SQL_SUCCEEDED(retcode)) {
        static SQLSMALLINT cols = 0;
        if (cols == 0) {
            retcode = SQLNumResultCols(hstmt, &cols);
            if (!SQL_SUCCEEDED(retcode)) {
                throw std::runtime_error("Failed at SQLNumResultCols " + std::to_string(retcode) + ".");
            }
        }
        std::vector<std::string> row;
        for (int i = 1; i <= cols; i++) {
            SQLCHAR data[BUF_LEN] = {0};
            SQLLEN indicator = 0;
            retcode = SQLGetData(hstmt, i, SQL_C_CHAR, &data, BUF_LEN, &indicator);
            if (!SQL_SUCCEEDED(retcode)) {
                throw std::runtime_error("Failed at SQLGetData " + std::to_string(retcode) + ".");
            }
            if (indicator != SQL_NULL_DATA) {
                std::string cell((const char *) data, indicator);
                row.push_back(cell);
            } else {
                row.push_back("<null>");
            }
        }
        return std::optional<std::vector<std::string>>{row};
    } else {
        return std::nullopt;
    }
}
