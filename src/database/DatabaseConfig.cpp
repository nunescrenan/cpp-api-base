#include "DatabaseConfig.hpp"
#include <cstdlib>
#include <string>

static std::string getEnv(const std::string& key, const std::string& defaultVal = "") {
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : defaultVal;
}

static int getEnvInt(const std::string& key, int defaultVal) {
    const char* val = std::getenv(key.c_str());
    return val ? std::stoi(val) : defaultVal;
}

DatabaseConfig DatabaseConfig::load() {
    DatabaseConfig config;
    config.driver = getEnv("DB_DRIVER", config.driver);
    config.host = getEnv("DB_HOST", config.host);
    config.port = getEnvInt("DB_PORT", config.port);
    config.user = getEnv("DB_USER", config.user);
    config.password = getEnv("DB_PASSWORD");
    config.name = getEnv("DB_NAME");
    return config;
}
