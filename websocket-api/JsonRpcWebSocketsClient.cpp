#include <thread>
#include <iostream>
#include "JsonRpcWebSocketsClient.h"
#include "JsonRpcHelper.h"
#include "JsonRpcMethods.h"

namespace WebsocketAPI {
JsonRpcWebSocketsClient::JsonRpcWebSocketsClient(BoostWebsocket& ws) : IWebsocketClient(ws)
{
}

void JsonRpcWebSocketsClient::readAsync(const std::string& jsonStr)
{
    std::thread{ std::bind(
            [client = shared_from_this(), json = jsonStr]() {
                try
                {
                    auto jsonObject = JsonRpcHelper::parse(json);
                    if(!JsonRpcHelper::validate(jsonObject))
                        return;

                    auto params = jsonObject["params"];
                    if(params.isArray()) {
                        std::cout << "for debugging" << std::endl;
                    }
                    auto test = params[0].asString();

                    auto base = std::static_pointer_cast<IWebsocketClient>(client);
                    JsonRpcMethods::invokeMethod(jsonObject, base.get());

                } catch (...)
                {
                    // TODO: log error here
                }
            }
    ) }.detach();
}

void JsonRpcWebSocketsClient::sendAsync(std::string jsonStr)
{
    std::thread{ std::bind(
            [this, json = jsonStr]() {
                sendSync(json);
            }
    ) }.detach();
}

void JsonRpcWebSocketsClient::sendSync(const std::string& jsonStr)
{
    try
    {
        beast::flat_buffer buffer;
        beast::ostream(buffer) << jsonStr;
        m_ws.write(buffer.data());

    } catch (std::exception& e)
    {
        // TODO: log error here
        std::cout << e.what() << std::endl;
    }
}

void JsonRpcWebSocketsClient::cacheSubscription(std::shared_ptr<Subscription> sub)
{
    m_subscriptionCache[sub->getType()] = sub;
}

}
