#include "CorsMiddleware.hpp"

void CorsMiddleware::invoke(const drogon::HttpRequestPtr& req, drogon::MiddlewareNextCallback&& nextCb, drogon::MiddlewareCallback&& mcb) {
    if (req->method() == drogon::Options) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        resp->addHeader("Access-Control-Max-Age", "86400");
        mcb(resp);
        return;
    }

    nextCb([mcb = std::move(mcb)](const drogon::HttpResponsePtr& resp) {
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        mcb(resp);
    });
}
