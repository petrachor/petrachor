
#include "SubsccriptionFactory.h"
#include "NewPendingTransactionSubscription.h"
#include "LogsSubscription.h"
#include "NewBlockHeader.h"
#include "SyncChangeSubscription.h"
#include <iostream>

namespace WebsocketAPI
{
    std::shared_ptr<Subscription> createSubscription(Json::Value json, IWebsocketClient* client)
    {
        // parse the params as defined here> https://geth.ethereum.org/docs/rpc/pubsub
        auto params = json["params"];
        auto subscriptionType = params[0].asString();

        auto evaluate = [=](const std::string& type){
            std::map<std::string, int> subscriptionMapping;
            subscriptionMapping.insert(
                    std::make_pair(std::string("newPendingTransactions"), Subscription::Type::NewPendingTransaction));
			subscriptionMapping.insert(
					std::make_pair(std::string("logs"), Subscription::Type::Logs));

			subscriptionMapping.insert(std::make_pair(std::string("newHeads"), Subscription::Type::NewBlockHeaders));
			subscriptionMapping.insert(std::make_pair(std::string("syncing"), Subscription::Type::SyncChanged));

            auto iter = subscriptionMapping.find(type);
            if(iter != subscriptionMapping.end())
                return iter->second;

            return -1;
        };
        auto type = evaluate(subscriptionType);

        switch(type) {
            case Subscription::Type::NewPendingTransaction:
                return std::make_shared<NewPendingTransactionSubscription>(client);
        	case Subscription::Type::Logs: {
				if( params.size() < 2) {
					//TODO: error here
					return nullptr;
				}

        		return std::make_shared<LogsSubscription>(client, params[1]);
			}
			case Subscription::Type::NewBlockHeaders:
				return std::make_shared<NewBlockHeader>(client);
        	case Subscription::Type::SyncChanged:
        		return std::make_shared<SyncChangeSubscription>(client);
        }

        return nullptr;
    }
}