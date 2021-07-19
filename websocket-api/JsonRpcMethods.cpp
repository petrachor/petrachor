#include <map>
#include <thread>
#include <iostream>
#include "JsonRpcMethods.h"
#include "websocket-api/subscription/SubsccriptionFactory.h"
#include "JsonRpcWebSocketsClient.h"

namespace WebsocketAPI { namespace JsonRpcMethods {

using RpcMethodMap = std::map<std::string, RpcMethod>;
static RpcMethodMap  f_methods;

void initialize()
{
    f_methods.clear();
    f_methods.insert(std::make_pair(std::string("eth_subscribe"), eth_subscribe));
	f_methods.insert(std::make_pair(std::string("eth_unsubscribe"), eth_unsubscribe));
}

bool find(const std::string& method)
{
    return get(method) != nullptr;
}

RpcMethod get(const std::string& method)
{
    RpcMethodMap::iterator it =  f_methods.find(method);
    if (it != f_methods.end()) {
        return it->second;
    }

    return nullptr;
}

void invokeMethod(Json::Value json, IWebsocketClient* client)
{
    auto name = json["method"].asString();
    auto method = get(name);
    if(method != nullptr) {
        method(json, client);
    }
}

void eth_subscribe(Json::Value json, IWebsocketClient* client)
{
	auto subscription = createSubscription(json, client);

    if(subscription != nullptr) {
        Json::Value root;

        root["jsonrpc"] = "2.0";
        root["id"] = 1;
        root["result"] = subscription->getId();

        Json::FastWriter fastWriter;
        std::string output = fastWriter.write(root);
        client->sendSync(output);

        //cache the subscription so it wont get freed
        JsonRpcWebSocketsClient* derived = (JsonRpcWebSocketsClient*)client;

        derived->cacheSubscription(subscription);
    }

    //TODO: send errors when method is not found
}

void eth_unsubscribe(Json::Value json, IWebsocketClient* client)
{
	//{"id": 1, "method": "eth_unsubscribe", "params": ["0xcd0c3e8af590364c09d0fa6a1210faf5"]}

	auto params = json["params"];
	auto subId = params[0].asString();

	JsonRpcWebSocketsClient* derived = (JsonRpcWebSocketsClient*)client;
	auto result = derived->unsubscribe(subId);

	Json::Value root;

	root["jsonrpc"] = "2.0";
	root["id"] = 1;
	root["result"] = result;

	Json::FastWriter fastWriter;
	std::string output = fastWriter.write(root);
	client->sendSync(output);

}

}}
