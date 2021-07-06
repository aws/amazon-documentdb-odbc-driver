#include <iostream>
#include <string>
#include <vector>
#include "client.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Usage: ODBCcli <connection-string> <query-statement>" << std::endl;
        std::cout << "Example: ODBCcli \"DSN=<your-dsn>\" \"select * from \"database\".\"table\"\"" << std::endl;
        return -1;
    }
    std::cout << "Connection string: " << argv[1] << std::endl;
    std::cout << "Query statement: " << argv[2] << std::endl;
    try {
        Client odbccli(argv[1]);
        odbccli.Query(argv[2]);
        int cnt = 0;
        std::optional<std::vector<std::string>> row;
        while ((row = odbccli.FetchOne()) != std::nullopt) {
            for (auto cell : row.value()) {
                std::cout << cell << "|";
            }
            std::cout << std::endl;
            cnt++;
        }
        std::cout << "Total rows: " << cnt << std::endl;
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
