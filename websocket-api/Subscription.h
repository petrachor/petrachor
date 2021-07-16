#pragma once
#include <string>
#include <memory>
#include "IWebSocketClient.h"

namespace WebsocketAPI {
    class Subscription{
    public:
        enum Type {
            NewPendingTransaction = 0
        };

    public:
        Subscription(IWebsocketClient* client, Type type);

        std::string getId();
        Type getType();
        IWebsocketClient* getClient();

    protected:
        std::string m_id;
        //std::shared_ptr<IWebsocketClient>& m_client;
        IWebsocketClient* m_client;
        Type m_type;
    };
}