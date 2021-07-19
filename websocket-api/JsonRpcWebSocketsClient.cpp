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
	m_subscriptionMap.insert(std::make_pair(sub->getId(), sub));
}

void JsonRpcWebSocketsClient::freeSubscription(SubscriptionMapIt iter)
{
	iter->second->cleanUp();
	iter->second.reset();
	m_subscriptionMap[iter->first] = nullptr;
}

bool JsonRpcWebSocketsClient::unsubscribe(const std::string& subId)
{
	auto iter = m_subscriptionMap.find(subId);
	if(iter == m_subscriptionMap.end())
		return false;

	freeSubscription(iter);
	m_subscriptionMap.erase(iter);
	return true;
}

void JsonRpcWebSocketsClient::close()
{
	SubscriptionMapIt iter = m_subscriptionMap.begin();
	while(iter != m_subscriptionMap.end()) {
		freeSubscription(iter);
		iter++;
	}

	m_subscriptionMap.clear();
}

//TODO: add unsubsscribe
}
