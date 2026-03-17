#pragma once

struct Config {
    int port = 9001;
    bool corsEnabled = true;

    static Config load();
};
