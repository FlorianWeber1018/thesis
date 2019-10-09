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
#include <queue>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

enum wsEvent{
    wsEvent_invalid = 0,
    wsEvent_dataNodeChange = 1,
    wsEvent_paramNodeChange = 2,
    wsEvent_pageChange = 3,
    wsEvent_structure = 4,
    wsEvent_authentification = 5};
struct ws_message{
    ws_message(const ws_message& msg);
    ws_message(const std::string& str);
    ws_message();
    wsEvent event = wsEvent_invalid;
    std::vector<std::string> payload;
    void to_Str(std::string& out);
    std::string to_Str();
};
class WebsocketServer; // FWD Declaration

void fail(beast::error_code ec, char const* what);

class ws_session : public std::enable_shared_from_this<ws_session>
{
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
    beast::flat_buffer buffer_in;


    const size_t outQueueMaxSize = 500;
public:
    // Take ownership of the socket
    ws_session(tcp::socket&& socket, ssl::context& ctx, WebsocketServer* websocketServer);
    ~ws_session();
    // Start the asynchronous operation
    void run();
    void on_handshake(beast::error_code ec);
    void on_accept(beast::error_code ec);
    //void do_read();// Read a ws_message into our buffer
    void after_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);

    void on_send(std::shared_ptr<ws_message> msg, bool filterEn);
    void send(std::shared_ptr<ws_message>& msg);//interface for flushing ws_messages
    void sendFiltered(std::shared_ptr<ws_message>& msg);//interface for flushing ws_messages
    bool checkDataNodeSubscription(const std::string& sqlId);
    bool checkParamNodeSubscription(const std::string& sqlId);
    void setAuthenticated();
private:
    void asyncReading();

    void dispatch(const ws_message& msg, std::shared_ptr<ws_session> ws_session_);


    bool authenticated_m = false;
    std::queue<std::shared_ptr<ws_message> > outQueue;
    std::set<std::string> dnSubscriptions;
    std::set<std::string> paramSubscriptions;
    std::mutex mutex_m;
    WebsocketServer* websocketServer_m = nullptr; //Pointer to underlieing WebsocketServer Object
};

// Accepts incoming connections and launches the ws_sessions
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
    friend class listener;
    friend class ws_session;
public:
    WebsocketServer();
    ~WebsocketServer();
protected:
    void addSession(std::shared_ptr<ws_session> ws_session_p);
    void removeDeletedSessions();
    void publishtoAllSessions(const ws_message &msg);
    virtual void ws_dispatch(const ws_message& msg, std::shared_ptr<ws_session> ws_session_) = 0;
private:
    std::list<std::weak_ptr<ws_session>> ws_sessions_m;
    std::mutex ws_sessionsMutex_m;
    net::ip::address const address = net::ip::make_address("0.0.0.0");
    unsigned short const port = static_cast<unsigned short>(18080);
    int const threadCnt = 1;

    std::shared_ptr<listener> listener_m;

    // The io_context is required for all I/O
    net::io_context ioc{threadCnt};

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12};

    // The Threads where running the io Service
    std::vector<std::thread> ioThreads;




};





#endif
