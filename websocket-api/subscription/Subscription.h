#pragma once
#include <string>
#include <memory>
#include "websocket-api/IWebSocketClient.h"
#include "boost/signals2/signal.hpp"

namespace WebsocketAPI {
    class Subscription{
    public:
        enum Type {
            NewPendingTransaction = 0,
            Logs = 1,
            NewBlockHeaders = 2,
            SyncChanged = 3
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
        boost::signals2::connection m_connection;
    };

	using SubscriptionPtr = std::shared_ptr<Subscription>;
}