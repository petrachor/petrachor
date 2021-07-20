#pragma once

#include <string>
#include <json/json.h>
#include "libethcore/BlockHeader.h"
#include "websocket-api/IWebSocketClient.h"
#include "Subscription.h"

namespace WebsocketAPI
{
	class NewBlockHeader : public Subscription{
	public:
		NewBlockHeader(IWebsocketClient* client);
		void onNewBlockHeader(dev::eth::BlockHeader bh);

	};
}