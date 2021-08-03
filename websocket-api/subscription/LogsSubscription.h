#pragma once
#include <string>
#include <json/json.h>

#include "websocket-api/IWebSocketClient.h"
#include "Subscription.h"
#include "libethereum/Transaction.h"

namespace WebsocketAPI
{
	class LogsSubscription : public Subscription{
	public:
		LogsSubscription(IWebsocketClient* client, Json::Value);
		void onBlocksMined();
		virtual void cleanUp();

	private:
		std::string m_filterId;
	};
}