#pragma once
#include <vector>
#include "IWebSocketClient.h"
#include "websocket-api/subscription/Subscription.h"

namespace WebsocketAPI {

    class JsonRpcWebSocketsClient : public IWebsocketClient,
            public std::enable_shared_from_this<JsonRpcWebSocketsClient> {

    public:
        JsonRpcWebSocketsClient(BoostWebsocket& ws);

        virtual void readAsync(const std::string& jsonStr);
        virtual void sendAsync(std::string jsonStr);
        virtual void sendSync(const std::string& jsonStr);
		virtual void close();

        void cacheSubscription(std::shared_ptr<Subscription> sub);
		bool unsubscribe(const std::string& subId);

    private:
		using SubscriptionMapIt = std::map<std::string, SubscriptionPtr>::iterator ;
		std::map<std::string, SubscriptionPtr> m_subscriptionMap;

		void freeSubscription(SubscriptionMapIt it);
		std::recursive_mutex m_mutex;
    };
}