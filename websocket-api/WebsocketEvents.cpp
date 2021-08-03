
#include "WebsocketEvents.h"

using namespace dev::eth;

namespace WebsocketAPI {
    std::shared_ptr<WebSocketEvents> WebSocketEvents::m_instance = nullptr;

    WebSocketEvents::WebSocketEvents()
    {}

    std::shared_ptr<WebSocketEvents>& WebSocketEvents::getInstance()
    {
        if(m_instance == nullptr)
            m_instance = std::make_shared<WebSocketEvents>();

        return m_instance;
    }

	boost::signals2::connection WebSocketEvents::subscribeNewPendingTransactionEvent(OnNewPendingTransactionEvent func)
    {
        return m_onPendingTransactionEvent.connect(func);
    }

    void WebSocketEvents::triggerNewPendingTransactionEvent(Transaction const _t)
    {
        m_onPendingTransactionEvent(_t);
    }

	boost::signals2::connection WebSocketEvents::subscribeBlocksMinedEvent(OnBlocksMined func)
	{
		return m_onBlocksMinedEvent.connect(func);
	}

    void WebSocketEvents::triggerBlocksMinedEvent()
	{
		m_onBlocksMinedEvent();
	}

	boost::signals2::connection WebSocketEvents::subscribeNewBlockHeaderEvent(OnNewBlockHeader func)
	{
    	return m_onNewBlockHeaderEvent.connect(func);
	}

	void WebSocketEvents::triggerNewBlockHeaderEvent(BlockHeader bh)
	{
		m_onNewBlockHeaderEvent(bh);
	}

	boost::signals2::connection WebSocketEvents::subscribeSyncChangeEvent(OnSyncChangeEvent func)
	{
    	return m_onSyncChangeEvent.connect(func);
	}

	void WebSocketEvents::triggerSyncChangeEvent()
	{
		m_onSyncChangeEvent();
	}
}