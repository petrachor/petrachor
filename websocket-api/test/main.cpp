#include <libp2p/Host.h>
#include <memory>
#include <iostream>
#include <chrono>
#include <thread>
#include "../WebsocketServer.h"
#include "../WebsocketEvents.h"

int main()
{
    std::thread{ std::bind(
            [=]() {
                WebsocketAPI::connect(8545);
            }
    ) }.detach();

    using namespace std;
    this_thread::sleep_for(std::chrono::milliseconds(10000) );
    dev::eth::Transaction t;
    WebsocketAPI::WebSocketEvents::getInstance()->triggerNewPendingTransactionEvent(t);

    return 0;
}
