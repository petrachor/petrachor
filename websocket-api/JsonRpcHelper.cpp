#include "JsonRpcHelper.h"
#include "JsonRpcMethods.h"

namespace WebsocketAPI { namespace JsonRpcHelper {
    Json::Value parse(const std::string& input)
    {
        Json::Reader reader;
        Json::Value root;
        reader.parse(input, root);
        return root;
    }
}}