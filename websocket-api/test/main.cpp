#include <libp2p/Host.h>
#include <memory>
#include <iostream>
#include <chrono>
#include <thread>
#include "../WebsocketServer.h"
#include "../WebsocketEvents.h"
#include "../JsonRpcWebSocketsClient.h"

int main()
{
#if 0
    std::thread{ std::bind(
            [=]() {
                WebsocketAPI::connect(8545);
            }
    ) }.detach();

    using namespace std;
    this_thread::sleep_for(std::chrono::milliseconds(10000) );
    dev::eth::Transaction t;
    WebsocketAPI::WebSocketEvents::getInstance()->triggerNewPendingTransactionEvent(t);
#endif
	WebsocketAPI::connect(8546);

    return 0;
}
