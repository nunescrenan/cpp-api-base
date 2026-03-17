#include "OAuthMiddleware.hpp"
#include <json/json.h>
#include "database/DatabaseConfig.hpp"
#include "database/DatabaseProvider.hpp"

static std::shared_ptr<drogon::orm::DbClient> dbClient;

static void initDbClient() {
    if (!dbClient) {
        auto config = DatabaseConfig::load();
        auto provider = DatabaseProvider::create(config.driver);
        dbClient = provider->createClient(config);
    }
}

static std::pair<std::string, std::string> parseBasicAuth(const std::string& authHeader) {
    if (authHeader.size() < 6 || authHeader.substr(0, 6) != "Basic ") {
        return {"", ""};
    }

    std::string encoded = authHeader.substr(6);
    std::string decoded;

    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : encoded) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            decoded.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    size_t colonPos = decoded.find(':');
    if (colonPos == std::string::npos) {
        return {"", ""};
    }

    return {decoded.substr(0, colonPos), decoded.substr(colonPos + 1)};
}

static bool validateCredentials(const std::string& clientId, const std::string& clientSecret) {
    initDbClient();

    try {
        auto result = dbClient->execSqlSync(
            "SELECT client_secret, is_active FROM api_clients WHERE client_id = ?",
            clientId
        );

        if (result.empty()) {
            return false;
        }

        auto row = result[0];
        std::string storedSecret = row["client_secret"].as<std::string>();
        bool isActive = row["is_active"].as<bool>();

        return isActive && storedSecret == clientSecret;
    } catch (...) {
        return false;
    }
}

void OAuthMiddleware::invoke(const drogon::HttpRequestPtr& req,
                              drogon::MiddlewareNextCallback&& nextCb,
                              drogon::MiddlewareCallback&& mcb) {
    std::string authHeader = std::string(req->getHeader("Authorization"));

    if (authHeader.empty()) {
        Json::Value json;
        json["error"] = "Missing Authorization header";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->addHeader("WWW-Authenticate", "Basic realm=\"API\"");
        mcb(resp);
        return;
    }

    auto [clientId, clientSecret] = parseBasicAuth(authHeader);

    if (clientId.empty() || clientSecret.empty()) {
        Json::Value json;
        json["error"] = "Invalid Authorization format";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
        resp->setStatusCode(drogon::k401Unauthorized);
        mcb(resp);
        return;
    }

    if (!validateCredentials(clientId, clientSecret)) {
        Json::Value json;
        json["error"] = "Invalid client credentials";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
        resp->setStatusCode(drogon::k401Unauthorized);
        mcb(resp);
        return;
    }

    nextCb([mcb = std::move(mcb)](const drogon::HttpResponsePtr& resp) {
        mcb(resp);
    });
}

void setupOAuth(drogon::HttpAppFramework& app) {
    LOG_INFO << "Setting up OAuth middleware...";

    try {
        initDbClient();
        LOG_INFO << "OAuth DB client initialized";
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to init OAuth DB client: " << e.what();
        return;
    }

    app.registerPreRoutingAdvice(
        [](const drogon::HttpRequestPtr& req,
           drogon::AdviceCallback&& callback,
           drogon::AdviceChainCallback&& chain) {
            std::string path = req->path();

            if (path.rfind("/api/", 0) != 0) {
                chain();
                return;
            }

            if (req->method() == drogon::Options) {
                chain();
                return;
            }

            std::string authHeader = std::string(req->getHeader("Authorization"));

            LOG_INFO << "OAuth check: " << path << " auth: " << (authHeader.empty() ? "(empty)" : "present");

            if (authHeader.empty()) {
                Json::Value json;
                json["error"] = "Missing Authorization header";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
                resp->setStatusCode(drogon::k401Unauthorized);
                resp->addHeader("WWW-Authenticate", "Basic realm=\"API\"");
                callback(resp);
                return;
            }

            auto [clientId, clientSecret] = parseBasicAuth(authHeader);

            if (clientId.empty() || clientSecret.empty()) {
                Json::Value json;
                json["error"] = "Invalid Authorization format";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
                resp->setStatusCode(drogon::k401Unauthorized);
                callback(resp);
                return;
            }

            if (!validateCredentials(clientId, clientSecret)) {
                Json::Value json;
                json["error"] = "Invalid client credentials";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
                resp->setStatusCode(drogon::k401Unauthorized);
                callback(resp);
                return;
            }

            chain();
        }
    );
}
