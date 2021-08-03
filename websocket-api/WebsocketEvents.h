#pragma once

#include <memory>
#include "boost/signals2/signal.hpp"
#include "libethereum/Transaction.h"
#include "libethcore/BlockHeader.h"

namespace WebsocketAPI
{
using namespace dev::eth;
using OnNewPendingTransactionEvent = std::function<void(Transaction const _t)>;
using OnBlocksMined = std::function<void()>;
using OnNewBlockHeader = std::function<void(BlockHeader bh)>;
using OnSyncChangeEvent = std::function<void()>;

class WebSocketEvents
{
public:
    WebSocketEvents();
    static std::shared_ptr<WebSocketEvents>& getInstance();

	boost::signals2::connection subscribeNewPendingTransactionEvent(OnNewPendingTransactionEvent func);
    void triggerNewPendingTransactionEvent(Transaction const _t);

	boost::signals2::connection subscribeBlocksMinedEvent(OnBlocksMined func);
	void triggerBlocksMinedEvent();

	boost::signals2::connection subscribeNewBlockHeaderEvent(OnNewBlockHeader func);
	void triggerNewBlockHeaderEvent(dev::eth::BlockHeader bh);

	boost::signals2::connection subscribeSyncChangeEvent(OnSyncChangeEvent func);
	void triggerSyncChangeEvent();

private:
    static std::shared_ptr<WebSocketEvents> m_instance;
    boost::signals2::signal<void(Transaction const _t)> m_onPendingTransactionEvent;
	boost::signals2::signal<void()> m_onBlocksMinedEvent;
	boost::signals2::signal<void(BlockHeader h)> m_onNewBlockHeaderEvent;
	boost::signals2::signal<void()> m_onSyncChangeEvent;
};
}