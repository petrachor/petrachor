#pragma once
#include "libweb3jsonrpc/Eth.h"

namespace WebsocketAPI
{
    void connect(unsigned short port);
    void connectAsync(unsigned short port);

	void setEthInterface(dev::rpc::Eth* eth);
    dev::rpc::Eth* getEthInterface();

}