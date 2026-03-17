#include "MeController.hpp"
#include <json/json.h>

void registerMeRoutes() {
    drogon::app().registerHandler(
        "/api/me",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            Json::Value json;
            json["status"] = "authenticated";
            json["message"] = "You have access to protected resources";

            auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
            callback(resp);
        },
        {drogon::Get, drogon::Options}
    );
}
