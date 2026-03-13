#pragma once

#include <drogon/drogon.h>
#include <json/json.h>

namespace Response {

inline drogon::HttpResponsePtr success(const Json::Value& data, int status = 200) {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(data);
    resp->setStatusCode(static_cast<drogon::HttpStatusCode>(status));
    return resp;
}

inline drogon::HttpResponsePtr error(const std::string& message, int status = 400) {
    Json::Value json;
    json["success"] = false;
    json["error"] = message;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(static_cast<drogon::HttpStatusCode>(status));
    return resp;
}

inline drogon::HttpResponsePtr notFound(const std::string& message = "Not found") {
    return error(message, 404);
}

inline drogon::HttpResponsePtr unauthorized(const std::string& message = "Unauthorized") {
    return error(message, 401);
}

inline drogon::HttpResponsePtr serverError(const std::string& message = "Internal server error") {
    return error(message, 500);
}

}
