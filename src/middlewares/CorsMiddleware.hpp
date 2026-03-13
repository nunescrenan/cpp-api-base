#pragma once

#include <drogon/drogon.h>

class CorsMiddleware : public drogon::HttpMiddleware<CorsMiddleware> {
public:
    void invoke(const drogon::HttpRequestPtr& req, drogon::MiddlewareNextCallback&& nextCb, drogon::MiddlewareCallback&& mcb) override;
};
