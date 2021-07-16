
#include "WebsocketEvents.h"

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

    void WebSocketEvents::subscribeNewPendingTransactionEvent(OnNewPendingTransactionEvent func)
    {
        m_onPendingTransactionEvent.connect(func);
    }

    void WebSocketEvents::triggerNewPendingTransactionEvent(Transaction const _t)
    {
        m_onPendingTransactionEvent(_t);
    }
}