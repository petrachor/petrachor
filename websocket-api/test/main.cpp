#include <libp2p/Host.h>
#include <memory>
#include <iostream>
#include <chrono>
#include <thread>
#include "../WebsocketServer.h"
#include "../WebsocketEvents.h"
#include "../JsonRpcWebSocketsClient.h"
#include <libweb3jsonrpc/JsonCustomWriter.h>

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
	innerTransaction["100"] = Json::Value::null;
	innerTransaction["noce"] = 1000000;

	Json::CustomStreamWriterBuilder builder;
	const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

	writer->write(innerTransaction, &std::cout);

    return 0;
}
