#pragma once

#include <memory>
#include <json/json.h>
#include "Subscription.h"

namespace WebsocketAPI
{
    std::shared_ptr<Subscription> createSubscription(Json::Value json, IWebsocketClient* client);
}