#pragma once

#include <memory>
#include "boost/signals2/signal.hpp"
#include "libethereum/Transaction.h"

namespace WebsocketAPI
{
using namespace dev::eth;
using OnNewPendingTransactionEvent = std::function<void(Transaction const _t)>;
using OnBlocksMined = std::function<void()>;
class WebSocketEvents
{
public:
    WebSocketEvents();
    static std::shared_ptr<WebSocketEvents>& getInstance();

    void subscribeNewPendingTransactionEvent(OnNewPendingTransactionEvent func);
    void triggerNewPendingTransactionEvent(Transaction const _t);

	void subscribeBlocksMinedEvent(OnBlocksMined func);
	void triggerBlocksMinedEvent();

private:
    static std::shared_ptr<WebSocketEvents> m_instance;
    boost::signals2::signal<void(Transaction const _t)> m_onPendingTransactionEvent;
	boost::signals2::signal<void()> m_onBlocksMinedEvent;
};
}