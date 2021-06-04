#include <iostream>
#include <string>
#ifdef WIN32
#include <windows.h>
#endif
#include <sql.h>
#include <sqlext.h>

constexpr static int buflen = 1024;

int main() {
    SQLHENV   henv  = SQL_NULL_HENV;   // Environment
    SQLHDBC   hdbc  = SQL_NULL_HDBC;   // Connection handle
    SQLHSTMT  hstmt = SQL_NULL_HSTMT;  // Statement handle
    SQLRETURN retcode;

    SQLCHAR strConnIn[buflen], strConnOut[buflen];
    SQLSMALLINT lenConnOut;

    // Allocate the environment handle.
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

    // Set the version environment attribute.
    retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

    // Allocate the connection handle.
    retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

    // Connect
    strcpy((char*)strConnIn, "DSN=timestream-aws-profile");

    retcode = SQLDriverConnect(hdbc, NULL, strConnIn, SQL_NTS, strConnOut, buflen, &lenConnOut, SQL_DRIVER_COMPLETE);
    if (retcode == SQL_SUCCESS) {
        std::cout << "Connected" << std::endl;
    } else {
        std::cout << "Not connected" << std::endl;
    }

    // Query
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    std::string query = "select * from sampleDB.DevOps";
    std::cout << "Query statement: " << query << std::endl;
    retcode = SQLExecDirect(hstmt, (SQLCHAR *) query.c_str(), (SQLINTEGER)query.length());
    if (retcode == SQL_SUCCESS) {
        std::cout << "SQLExecDirect is okay!" << std::endl;
        SQLSMALLINT num_of_cols  = 0;
        SQLNumResultCols(hstmt, &num_of_cols);
        while ((retcode = SQLFetch(hstmt)) != SQL_NO_DATA) {
            if (SQL_SUCCEEDED(retcode)) {
                for (int i = 1; i <= num_of_cols; i++) {
                    SQLCHAR data[buflen] = {0};
                    SQLLEN indicator = 0;
                    retcode = SQLGetData(hstmt, i, SQL_C_CHAR, &data, buflen, &indicator);
                    if (retcode == SQL_SUCCESS) {
                        if (indicator != SQL_NULL_DATA) {
                            std::cout << data << "|";
                        } else {
                            std::cout << "<null>" << "|";
                        }
                    } else {
                        std::cout << "SQLGetData calls failed!" << std::endl;
                    }
                }
                std::cout << std::endl;
            }
        }
    } else {
        std::cout << "SQLExecDirect failed!" << std::endl;
    }

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
    return 0;
}
