#include "PingController.hpp"
#include <json/json.h>

void registerPingRoutes() {
    drogon::app().registerHandler(
        "/ping",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            Json::Value json;
            json["status"] = "200 OK";
            json["message"] = "pong";

            auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
            callback(resp);
        },
        {drogon::Get}
    );
}
