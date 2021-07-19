#pragma once
#include <string>
#include <functional>
#include <json/json.h>
#include "IWebSocketClient.h"

namespace WebsocketAPI { namespace JsonRpcMethods {

    using RpcMethod = std::function<void(Json::Value json, IWebsocketClient* client)>;

    void initialize();

    bool find(const std::string& method);

    RpcMethod get(const std::string& method);

    void invokeMethod(Json::Value json, IWebsocketClient* client);

    void eth_subscribe(Json::Value json, IWebsocketClient* client);
    void eth_unsubscribe(Json::Value json, IWebsocketClient* client);
}}
