#include <iostream>
#include <functional>
#include <json/json.h>
#include <thread>

#include "NewPendingTransactionSubscription.h"
#include "websocket-api/WebsocketEvents.h"

namespace WebsocketAPI {
    NewPendingTransactionSubscription::NewPendingTransactionSubscription(IWebsocketClient* client)
        : Subscription(client, Subscription::Type::NewPendingTransaction) {

        // subscribe to event
        WebSocketEvents::getInstance()->subscribeNewPendingTransactionEvent(
                std::bind(&NewPendingTransactionSubscription::onPendingTransaction, this, std::placeholders::_1));
    }

    void NewPendingTransactionSubscription::onPendingTransaction(dev::eth::Transaction const _t) {
     //   std::thread{ std::bind(
       //         [=]() {
                    try {
                        auto hash = _t.sha3();
                        auto transactiohHash = "0x" + hash.hex();

                        Json::Value root;
                        Json::Value params;

                        params["subscription"] = getId();
                        params["result"] = transactiohHash;

                        root["jsonrpc"] = "2.0";
                        root["method"] = "eth_subscription";
                        root["params"] = params;

                        Json::FastWriter fastWriter;
                        std::string output = fastWriter.write(root);

                        getClient()->sendSync(output);
                    } catch (std::exception &e) {
                        std::cout << e.what() << std::endl;
                    }
         //       }
       // ) }.detach();
    }
}