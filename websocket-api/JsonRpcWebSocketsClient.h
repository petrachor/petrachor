#pragma once
#include "IWebSocketClient.h"
#include "Subscription.h"

namespace WebsocketAPI {

    class JsonRpcWebSocketsClient : public IWebsocketClient,
            public std::enable_shared_from_this<JsonRpcWebSocketsClient> {

    public:
        JsonRpcWebSocketsClient(BoostWebsocket& ws);
        virtual void readAsync(const std::string& jsonStr);
        virtual void sendAsync(std::string jsonStr);
        virtual void sendSync(const std::string& jsonStr);

        void cacheSubscription(std::shared_ptr<Subscription> sub);

    private:
        std::shared_ptr<Subscription> m_subscriptionCache[3];
    };
}