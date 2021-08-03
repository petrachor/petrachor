#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

namespace WebsocketAPI {

    namespace beast = boost::beast;         // from <boost/beast.hpp>
    namespace http = beast::http;           // from <boost/beast/http.hpp>
    namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
    namespace net = boost::asio;            // from <boost/asio.hpp>
    using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
    using BoostWebsocket = websocket::stream<tcp::socket>;

    class IWebsocketClient{
    public:
        IWebsocketClient(BoostWebsocket& ws) : m_ws(ws){}

        virtual void readAsync(const std::string& jsonStr) = 0;
        virtual void sendAsync(std::string jsonStr) = 0;
        virtual void sendSync(const std::string& jsonStr) = 0;

        virtual void close() = 0;

    protected:
        BoostWebsocket& m_ws;
    };
}