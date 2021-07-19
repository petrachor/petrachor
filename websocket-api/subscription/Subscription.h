#pragma once
#include <string>
#include <memory>
#include "websocket-api/IWebSocketClient.h"

namespace WebsocketAPI {
    class Subscription{
    public:
        enum Type {
            NewPendingTransaction = 0,
            Logs = 1
        };

    public:
        Subscription(IWebsocketClient* client, Type type);

        std::string getId();
        Type getType();
        IWebsocketClient* getClient();
        virtual void cleanUp();

    protected:
        std::string m_id;
        IWebsocketClient* m_client;
        Type m_type;
    };

	using SubscriptionPtr = std::shared_ptr<Subscription>;
}