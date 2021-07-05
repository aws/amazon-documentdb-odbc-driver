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
        throw std::runtime_error("Failed at SQLAllocHandle SQL_HANDLE_ENV");
    }
    // Set the version environment attribute
    retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER *) SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(retcode)) {
        throw std::runtime_error("Failed at SQLSetEnvAttr SQL_ATTR_ODBC_VERSION");
    }
    // Allocate the connection handle
    retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (!SQL_SUCCEEDED(retcode)) {
        throw std::runtime_error("Failed at SQLAllocHandle SQL_HANDLE_DBC");
    }
    // Connect
    SQLCHAR strConnIn[BUF_LEN] = {0}, strConnOut[BUF_LEN] = {0};
    SQLSMALLINT lenConnOut;
    strcpy((char *) strConnIn, connection_string);
    retcode = SQLDriverConnect(hdbc, NULL, strConnIn, SQL_NTS, strConnOut, BUF_LEN, &lenConnOut, SQL_DRIVER_COMPLETE);
    if (!SQL_SUCCEEDED(retcode)) {
        throw std::runtime_error("Failed at SQLDriverConnect");
    }
    // Allocate the statement handle
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (!SQL_SUCCEEDED(retcode)) {
        throw std::runtime_error("Failed at SQLAllocHandle SQL_HANDLE_STMT");
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
        throw std::runtime_error("Failed at SQLExecDirect");
    }
}

std::optional<std::vector<std::string>> Client::FetchOne() {
    auto retcode = SQLFetch(hstmt);
    if (SQL_SUCCEEDED(retcode)) {
        static SQLSMALLINT cols = 0;
        if (cols == 0) {
            retcode = SQLNumResultCols(hstmt, &cols);
            if (!SQL_SUCCEEDED(retcode)) {
                throw std::runtime_error("Failed at SQLNumResultCols");
            }
        }
        std::vector<std::string> row;
        for (int i = 1; i <= cols; i++) {
            SQLCHAR data[BUF_LEN] = {0};
            SQLLEN indicator = 0;
            retcode = SQLGetData(hstmt, i, SQL_C_CHAR, &data, BUF_LEN, &indicator);
            if (!SQL_SUCCEEDED(retcode)) {
                throw std::runtime_error("Failed at SQLGetData");
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
