#include "Config.hpp"
#include <cstdlib>
#include <fstream>
#include <json/json.h>

std::string getEnv(const std::string& key, const std::string& defaultVal) {
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : defaultVal;
}

Config Config::load() {
    Config config;

    std::ifstream file("config.json");
    if (file.is_open()) {
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(file, root)) {
            if (root.isMember("host")) config.host = root["host"].asString();
            if (root.isMember("port")) config.port = root["port"].asInt();
            if (root.isMember("threads")) config.threads = root["threads"].asInt();
            if (root.isMember("logLevel")) config.logLevel = root["logLevel"].asString();
            if (root.isMember("corsEnabled")) config.corsEnabled = root["corsEnabled"].asBool();
        }
    }

    config.host = getEnv("HOST", config.host);
    config.port = std::stoi(getEnv("PORT", std::to_string(config.port)));
    config.threads = std::stoi(getEnv("THREADS", std::to_string(config.threads)));
    config.logLevel = getEnv("LOG_LEVEL", config.logLevel);

    return config;
}
