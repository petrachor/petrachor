#pragma once
#include "websocket-api/IWebSocketClient.h"
#include "Subscription.h"
#include "libethereum/Transaction.h"

namespace WebsocketAPI
{
    class NewPendingTransactionSubscription : public Subscription{
    public:
        NewPendingTransactionSubscription(IWebsocketClient* client);
        void onPendingTransaction(dev::eth::Transaction const _t);
    };
}