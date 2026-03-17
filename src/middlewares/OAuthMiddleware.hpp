#pragma once

#include <drogon/drogon.h>

class OAuthMiddleware : public drogon::HttpMiddleware<OAuthMiddleware> {
public:
    void invoke(const drogon::HttpRequestPtr& req,
                drogon::MiddlewareNextCallback&& nextCb,
                drogon::MiddlewareCallback&& mcb) override;
};

void setupOAuth(drogon::HttpAppFramework& app);
