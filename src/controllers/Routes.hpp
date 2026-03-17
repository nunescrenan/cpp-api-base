#pragma once

#include "ping/PingController.hpp"
#include "me/MeController.hpp"

inline void registerAllRoutes() {
    registerPingRoutes();
    registerMeRoutes();
}
