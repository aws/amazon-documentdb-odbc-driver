// SQLBindCol_ref.cpp  
// compile with: odbc32.lib  
#include <stdio.h>  
 
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <iostream>
#include <dlfcn.h>
#include <link.h>
  
#define NAME_LEN 50  
#define PHONE_LEN 60
  
void show_error() {  
   printf("error\n");  
}  
  
int main(int argc, char *argv[]) {  
   SQLHENV henv;  
   SQLHDBC hdbc;  
   SQLHSTMT hstmt = 0;  
   SQLRETURN retcode;  
   SQLWCHAR szName[NAME_LEN], szPhone[PHONE_LEN], sCustID[NAME_LEN];  
   SQLLEN cbName = 0, cbCustID = 0, cbPhone = 0;  
   int limit = 1;
   if (argc >1) {
       limit = atoi(argv[1]);
   }
    for ( int i =0; i < limit ; i++ ) {
            void *handle;

        // Allocate environment handle  
        retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);  
        
        // Set the ODBC version environment attribute  
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
            retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);   
        
            // Allocate connection handle  
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
                retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);  
        
                // Set login timeout to 5 seconds  
                if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
                    //SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);  
        
                    // std::string connectionString =
                    // "DRIVER={Amazon DocumentDB};"
                    // "HOSTNAME="  "localhost"  ":"  "27017"  ";"
                    // "DATABASE="  "odbc-test"  ";"
                    // "USER="  "documentdb"  ";"
                    // "PASSWORD="  "bqdocumentdblab"  ";"
                    // "TLS=true;"
                    // "TLS_ALLOW_INVALID_HOSTNAMES=true;";

                    SQLCHAR connectStr[] =
                        "DRIVER={Amazon DocumentDB};HOSTNAME=host.docker.internal:27017;DATABASE=odbc-test;USER=documentdb;PASSWORD=bqdocumentdblab;TLS=false;";


                    // Connect to data source  
                    //retcode = SQLConnect(hdbc, (SQLWCHAR*) L"Amazon DocumentDB", SQL_NTS, (SQLWCHAR*) NULL, 0, NULL, 0);  

                    SQLCHAR outstr[1024];
                    SQLSMALLINT outstrlen;

                    // Connecting to ODBC server.
                    // retcode =
                    //     SQLDriverConnect(hdbc, NULL, (SQLWCHAR*) connectStr,
                    //                     static_cast< SQLSMALLINT >(sizeof(connectStr)), (SQLWCHAR*) outstr,
                    //                     sizeof(outstr), &outstrlen, SQL_DRIVER_COMPLETE);

                    retcode = SQLDriverConnect(hdbc, NULL, connectStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

                    // error handling
                    if ( retcode != SQL_SUCCESS) {
                        SQLCHAR sqlstate[7] = {};
                        SQLINTEGER nativeCode;
                        SQLCHAR message[1024];
                        SQLSMALLINT reallen = 0;

                        SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, sqlstate, &nativeCode, message,
                                        1024, &reallen);

                        std::string stringsqlstate = std::string(reinterpret_cast< char* >(sqlstate));
                        std::string stringmessage = std::string(reinterpret_cast< char* >(message), reallen);
                        std::cout << stringsqlstate << std::endl;
                        std::cout << stringmessage << std::endl;
                    }
        
                    // Allocate statement handle  
                    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {   
                    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);  
                    SQLCHAR query[] =  "SELECT * FROM queries_test_001 ";
                    retcode = SQLExecDirect(hstmt, query, SQL_NTS); 
                    std::cout << "retcode execdirect " << retcode << std::endl; 
                    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
        /*
        
                        // Bind columns 1, 2, and 3  
                        retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &sCustID, 100, &cbCustID);  
                        retcode = SQLBindCol(hstmt, 2, SQL_C_WCHAR, szName, NAME_LEN, &cbName);  
                        retcode = SQLBindCol(hstmt, 3, SQL_C_WCHAR, szPhone, PHONE_LEN, &cbPhone);   
        
                        // Fetch and print each row of data. On an error, display a message and exit.  
                        for (int i=0 ; ; i++) {  
                            retcode = SQLFetch(hstmt);  
                            if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)  
                                show_error();  
                            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)  
                            {
                                //replace wprintf with printf
                                //%S with %ls
                                //warning C4477: 'wprintf' : format string '%S' requires an argument of type 'char *'
                                //but variadic argument 2 has type 'SQLWCHAR *'
                                //wprintf(L"%d: %S %S %S\n", i + 1, sCustID, szName, szPhone);  
                                printf("%d: %ls %ls %ls\n", i + 1, sCustID, szName, szPhone);  
                            }    
                            else  
                                break;  
                        }  
        */
                    }  


                    // Process data  
                    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
                        SQLCancel(hstmt);  
                        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
                        hstmt = 0;  
                    }  
        
                    SQLDisconnect(hdbc);  
                    
                    }  
        
                    SQLFreeHandle(SQL_HANDLE_DBC, hdbc); 
                    hdbc = 0; 
                }  
            }  
            SQLFreeHandle(SQL_HANDLE_ENV, henv);  
            henv = 0;
        }

        std::cout << "i " << i << std::endl;  
    }
    using UnknownStruct = struct unknown_struct {
   void*  pointers[3];
   struct unknown_struct* ptr;
};
using LinkMap = struct link_map;

auto* handle = dlopen(NULL, RTLD_NOW);
auto* p = reinterpret_cast<UnknownStruct*>(handle)->ptr;
auto* map = reinterpret_cast<LinkMap*>(p->ptr);

while (map) {
  std::cout << map->l_name << std::endl;
  std::cout << map->l_addr << std::endl;
  // do something with |map| like with handle, returned by |dlopen()|.
  map = map->l_next;
}
}