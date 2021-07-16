#pragma once
#include <json/json.h>
#include <string>

namespace WebsocketAPI { namespace JsonRpcHelper {
    Json::Value parse(const std::string& input);

    bool validate(const Json::Value& json);
}}