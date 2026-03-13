#pragma once

#include <string>

struct Config {
    std::string host = "127.0.0.1";
    int port = 9001;
    int threads = 1;
    std::string logLevel = "info";
    bool corsEnabled = true;

    static Config load();
};
