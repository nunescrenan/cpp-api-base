#pragma once

#include "../DatabaseProvider.hpp"
#include <format>

class MySqlProvider : public DatabaseProvider {
public:
    std::shared_ptr<drogon::orm::DbClient> createClient(
        const DatabaseConfig& config,
        size_t connectionCount = 1
    ) override {
        std::string connStr = std::format(
            "host={} port={} dbname={} user={} password={}",
            config.host,
            config.port,
            config.name,
            config.user,
            config.password
        );

        return drogon::orm::DbClient::newMysqlClient(connStr, connectionCount);
    }

    std::string getName() const override {
        return "mysql";
    }
};
