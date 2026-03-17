#include "Config.hpp"
#include <cstdlib>
#include <string>

static std::string getEnv(const std::string& key, const std::string& defaultVal) {
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : defaultVal;
}

static bool getEnvBool(const std::string& key, bool defaultVal) {
    const char* val = std::getenv(key.c_str());
    if (!val) return defaultVal;
    std::string s(val);
    return s == "true" || s == "1" || s == "yes";
}

Config Config::load() {
    Config config;
    config.port = std::stoi(getEnv("PORT", std::to_string(config.port)));
    config.corsEnabled = getEnvBool("CORS_ENABLED", config.corsEnabled);
    return config;
}
