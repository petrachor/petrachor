#include <memory>
#include "boost/signals2/signal.hpp"
#include "libethereum/Transaction.h"

namespace WebsocketAPI
{
using namespace dev::eth;
using OnNewPendingTransactionEvent = std::function<void(Transaction const _t)>;

class WebSocketEvents
{
public:
    WebSocketEvents();
    static std::shared_ptr<WebSocketEvents>& getInstance();

    void subscribeNewPendingTransactionEvent(OnNewPendingTransactionEvent func);
    void triggerNewPendingTransactionEvent(Transaction const _t);

private:
    static std::shared_ptr<WebSocketEvents> m_instance;
    boost::signals2::signal<void(Transaction const _t)> m_onPendingTransactionEvent;
};
}