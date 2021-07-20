#include "libweb3jsonrpc/JsonHelper.h"
#include "NewBlockHeader.h"
#include "../WebsocketEvents.h"

using namespace dev::eth;

namespace WebsocketAPI
{
	NewBlockHeader::NewBlockHeader(IWebsocketClient* client)
		: Subscription(client, Subscription::Type::NewBlockHeaders)
	{
		m_connection = WebSocketEvents::getInstance()->subscribeNewBlockHeaderEvent(
				std::bind(&NewBlockHeader::onNewBlockHeader, this, std::placeholders::_1));
	}

	void NewBlockHeader::onNewBlockHeader(BlockHeader bh)
	{
		Json::Value root;
		Json::Value params;
		Json::Value result;

		params["subscription"] = getId();

		root["jsonrpc"] = "2.0";
		root["method"] = "eth_subscription";

		result = toJson(bh);
		params["subscription"] = getId();
		params["result"] = result;

		root["params"] = params;

		Json::FastWriter fastWriter;
		std::string out = fastWriter.write(root);
		getClient()->sendSync(out);
	}
}