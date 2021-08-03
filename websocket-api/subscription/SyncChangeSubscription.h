#pragma once

#include <string>
#include <json/json.h>
#include "libethcore/BlockHeader.h"
#include "websocket-api/IWebSocketClient.h"
#include "Subscription.h"

namespace WebsocketAPI
{
	class SyncChangeSubscription : public Subscription{
	public:
		SyncChangeSubscription(IWebsocketClient* client);
		void onSyncChanged();

	};
}