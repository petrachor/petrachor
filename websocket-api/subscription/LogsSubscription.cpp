#include <websocket-api/WebsocketEvents.h>
#include "libweb3jsonrpc/Eth.h"
#include "LogsSubscription.h"
#include "../WebsocketServer.h"

namespace WebsocketAPI
{
	LogsSubscription::LogsSubscription(IWebsocketClient* client, Json::Value j)
		: Subscription(client, Subscription::Type::Logs) {

		auto interface = WebsocketAPI::getEthInterface();
		if(interface == nullptr)
			return;

		m_filterId = interface->eth_newFilterWS(j);

		WebSocketEvents::getInstance()->subscribeBlocksMinedEvent(
				std::bind(&LogsSubscription::onBlocksMined, this));
	}

	void LogsSubscription::onBlocksMined()
	{
		try {
			auto interface = WebsocketAPI::getEthInterface();
			if (interface == nullptr || m_filterId.empty())
				return;

			Json::Value jlogs = interface->eth_getFilterChanges(m_filterId);

			Json::FastWriter fastWriter;
			Json::Value root;
			Json::Value params;

			params["subscription"] = getId();

			root["jsonrpc"] = "2.0";
			root["method"] = "eth_subscription";

			for (unsigned int i = 0; i < jlogs.size(); i++)
			{
				if(jlogs[i]["type"].asString() != "mined")
					continue;

				params["result"] = jlogs[i];
				root["params"] = params;

				std::string result = fastWriter.write(root);
				getClient()->sendSync(result);
			}

		} catch (std::exception &e) {
			std::cout << e.what() << std::endl;
		}
	}

	void LogsSubscription::cleanUp()
	{
		auto interface = WebsocketAPI::getEthInterface();
		if (interface == nullptr || m_filterId.empty())
			return;

		interface->eth_uninstallFilterWS(m_filterId);
	}
}