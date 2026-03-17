#pragma once

#include <string>

struct DatabaseConfig {
    std::string driver = "mysql";
    std::string host = "localhost";
    int port = 3306;
    std::string user = "root";
    std::string password;
    std::string name;

    static DatabaseConfig load();
};
