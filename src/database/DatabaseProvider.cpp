#include "DatabaseProvider.hpp"
#include "mysql/MySqlProvider.hpp"
#include <stdexcept>

std::unique_ptr<DatabaseProvider> DatabaseProvider::create(const std::string& driver) {
    if (driver == "mysql") {
        return std::make_unique<MySqlProvider>();
    }

    throw std::runtime_error("Unsupported database driver: " + driver);
}
