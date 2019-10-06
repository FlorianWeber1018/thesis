#ifndef websocketServer__hpp
#define websocketServer__hpp
#include "globalInclude.hpp"
#include "websocketServer.hpp"
#include "certificates/server_certificate.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
enum websocketEvent{DataChangeRequest = 0 };
class WebsocketServer; // FWD Declaration
    void fail(beast::error_code ec, char const* what);

    class session : public std::enable_shared_from_this<session>
    {
        websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
        beast::flat_buffer buffer_;

    public:
        // Take ownership of the socket
        session(tcp::socket&& socket, ssl::context& ctx/*, WebsocketServer* websocketServer*/);

        // Start the asynchronous operation
        void run();
        void on_handshake(beast::error_code ec);
        void on_accept(beast::error_code ec);
        void do_read();// Read a message into our buffer
        void on_read(beast::error_code ec, std::size_t bytes_transferred);
        void on_write(beast::error_code ec, std::size_t bytes_transferred);
    private:
        WebsocketServer* websocketServer_m = nullptr; //Pointer to underlieing WebsocketServer Object
    };
    // Accepts incoming connections and launches the sessions
    class listener : public std::enable_shared_from_this<listener>
    {
        net::io_context& ioc_;
        ssl::context& ctx_;
        tcp::acceptor acceptor_;

    public:
        listener(net::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, WebsocketServer* websocketServer);

        void run(); // Start accepting incoming connections

    private:
        WebsocketServer* websocketServer_m = nullptr; //Pointer to underlieing Websocket Server Object
        void do_accept();
        void on_accept(beast::error_code ec, tcp::socket socket);
    };

    class WebsocketServer
    {
    public:
        WebsocketServer();
        ~WebsocketServer();
    protected:
        //virtual void dispatcher(); //dispatcher to be overloaded in Backend to transmit events to the OPCUAServer and SQLClient class
    private:
        net::ip::address const address = net::ip::make_address("0.0.0.0");
        unsigned short const port = static_cast<unsigned short>(18080);
        int const threads = 1;

        std::shared_ptr<listener> listener_m;

        // The io_context is required for all I/O
        net::io_context ioc{threads};

        // The SSL context is required, and holds certificates
        ssl::context ctx{ssl::context::tlsv12};

        // The Threads where running the io Service
        std::vector<std::thread> ioThreads;
    };





#endif
