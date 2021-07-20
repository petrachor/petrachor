#include "SyncChangeSubscription.h"
#include "../WebsocketEvents.h"
#include "../WebsocketServer.h"

namespace WebsocketAPI
{
	SyncChangeSubscription::SyncChangeSubscription(IWebsocketClient* client)
			: Subscription(client, Subscription::SyncChanged)
	{
		m_connection = WebSocketEvents::getInstance()->subscribeSyncChangeEvent(
				std::bind(&SyncChangeSubscription::onSyncChanged, this));
	}

	void SyncChangeSubscription::onSyncChanged()
	{
		auto interface = WebsocketAPI::getEthInterface();
		if(interface == nullptr)
			return;

		auto result = interface->eth_syncing();
		Json::Value root;
		Json::Value params;

		params["subscription"] = getId();

		root["jsonrpc"] = "2.0";
		root["method"] = "eth_subscription";
		params["subscription"] = getId();
		params["result"] = result;

		root["params"] = params;

		Json::FastWriter fastWriter;
		std::string out = fastWriter.write(root);
		getClient()->sendSync(out);
	}
}