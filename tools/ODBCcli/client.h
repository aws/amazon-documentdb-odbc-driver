#ifndef ODBCCLI_CLIENT_H
#define ODBCCLI_CLIENT_H

#ifdef WIN32

#include <windows.h>

#endif

#include <sql.h>
#include <sqlext.h>
#include <vector>
#include <string>
#include <optional>

class Client {
public:
    static constexpr int BUF_LEN = 1024;

    explicit Client(const char *connection_string);

    ~Client();

    void Query(const char *query);

    std::optional<std::vector<std::string>> FetchOne();

private:
    SQLHENV henv;   // Environment
    SQLHDBC hdbc;   // Connection handle
    SQLHSTMT hstmt;  // Statement handle
};


#endif //ODBCCLI_CLIENT_H
