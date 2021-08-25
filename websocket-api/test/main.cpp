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
//	WebsocketAPI::connect(8546);

	Json::Value innerTransaction;
	innerTransaction["from"] = "0x0216d5032f356960cd3749c31ab34eeff21b3395";
	innerTransaction["blockHash"] = "0x0000000000000000000000000000000000000000000000000000000000000000";

	int key = 0x326;
	std::string nonce = std::to_string(key);
	Json::Value nonceJson;
	nonceJson[nonce] = innerTransaction;

	Json::Value account1;
	std::string a1Str = "0x0216d5032f356960cd3749c31ab34eeff21b3395";
	std::string a2Str = "0x24d407e5a0b506e1cb2fae163100b5de01f5193c";
	account1[a1Str] = nonceJson;
	account1[a2Str] = nonceJson;

	Json::Value pending;
	pending["pending"] = account1;

	Json::FastWriter fastWriter;
	std::string output = fastWriter.write(pending);

	std::cout << output;

    return 0;
}
