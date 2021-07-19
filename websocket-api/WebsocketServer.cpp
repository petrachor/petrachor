#include <iostream>
#include <string>
#include <thread>
#include <memory.h>
#include "JsonRpcWebSocketsClient.h"
#include "JsonRpcMethods.h"
#include "WebsocketServer.h"

namespace WebsocketAPI
{

static dev::rpc::Eth* f_ethInterface = nullptr;

void setEthInterface(dev::rpc::Eth* eth)
{
	f_ethInterface = eth;
}

dev::rpc::Eth* getEthInterface()
{
	return f_ethInterface;
}

void connect(unsigned short port)
{
    JsonRpcMethods::initialize();

    auto const address = net::ip::make_address("127.0.0.1");
    net::io_context ioc{ 1 };
    tcp::acceptor acceptor{ ioc, {address, port} };
    for (;;)
    {

        tcp::socket socket{ ioc };

        acceptor.accept(socket); // stops until new socket/client connections

        std::thread{ std::bind(
                [q{std::move(socket)}]() { // socket will be const - mutable should be used

                    websocket::stream<tcp::socket> ws{std::move(const_cast<tcp::socket&>(q))};

                    // Set a decorator to change the Server of the handshake
                    // no need to set. It ıs not necessary
                    ws.set_option(websocket::stream_base::decorator(
                            [](websocket::response_type& res)
                            {
                                res.set(http::field::server,
                                        std::string(BOOST_BEAST_VERSION_STRING) +
                                        " websocket-server-sync");
                            }));

                    // Accept the websocket handshake
                    ws.accept();

                    std::shared_ptr<JsonRpcWebSocketsClient> client = std::make_shared<JsonRpcWebSocketsClient>(ws);

                    while (true)
                    {
                        try
                        {

                            // This buffer will hold the incoming message
                            // buffer types https://www.boost.org/doc/libs/1_75_0/libs/beast/doc/html/beast/using_io/buffer_types.html
                            // check for the best one
                            //beast::multi_buffer buffer;
                            beast::flat_buffer buffer;

                            // Read a message
                            ws.read(buffer);   // synchronous hold until new data

                            auto out = beast::buffers_to_string(buffer.cdata());
                            std::cout << out << std::endl;		// for debugging on data received by websocket server

                            client->readAsync(out);

                        }
                        catch (beast::system_error const& se)
                        {
                            if (se.code() != websocket::error::closed)
                            {
                                std::cerr << "Error: " << se.code().message() << std::endl;
                                break;
                            }
                        }
                    } // end of while
                    client->close();
                }
        ) }.detach();
    }
}

void connectAsync(unsigned short port){
    std::thread{ std::bind(
            [=]() {
              connect(port);
            }
    ) }.detach();
}


}