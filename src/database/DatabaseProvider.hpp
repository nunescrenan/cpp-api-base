#pragma once

#include <drogon/orm/DbClient.h>
#include <memory>
#include <string>
#include "DatabaseConfig.hpp"

class DatabaseProvider {
public:
    virtual ~DatabaseProvider() = default;

    virtual std::shared_ptr<drogon::orm::DbClient> createClient(
        const DatabaseConfig& config,
        size_t connectionCount = 1
    ) = 0;

    virtual std::string getName() const = 0;

    static std::unique_ptr<DatabaseProvider> create(const std::string& driver);
};
