
#include "SubsccriptionFactory.h"
#include "NewPendingTransactionSubscription.h"
#include <iostream>

namespace WebsocketAPI
{
    std::shared_ptr<Subscription> createSubscription(Json::Value json, IWebsocketClient* client)
    {
        // parse the params as defined here> https://geth.ethereum.org/docs/rpc/pubsub
        auto params = json["params"];
        auto subscriptionType = params[0].asString();

        constexpr int NewPendingTransactionType = 0;

        auto evaluate = [=](const std::string& type){
            std::map<std::string, int> subscriptionMapping;
            subscriptionMapping.insert(
                    std::make_pair(std::string("newPendingTransactions"), NewPendingTransactionType));

            auto iter = subscriptionMapping.find(type);
            if(iter != subscriptionMapping.end())
                return iter->second;

            return -1;
        };
        auto type = evaluate(subscriptionType);

        switch(type) {
            case NewPendingTransactionType:
                return std::make_shared<NewPendingTransactionSubscription>(client);
        }

        return nullptr;
    }
}