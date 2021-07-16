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

    bool validate(const Json::Value& json)
    {
        const std::string method = json["method"].asString();
        if(method.empty()) {
            //TODO: log proper error
            return false;
        }

        if(!JsonRpcMethods::find(method))
            return false;

        return true;
    }
}}