#include "websocketServer.hpp"
#include "globalInclude.hpp"
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


void ws_message::to_Str(std::string& out)
{
    out.append(std::to_string(event));
    for(const auto& element : payload){
        out.append(";");
        out.append(element);
    }
}
std::string ws_message::to_Str(){
    std::string str;
    to_Str(str);
    return str;
}
ws_message::ws_message(const ws_message& msg) : payload(msg.payload)
{
    this->event = msg.event;
}
ws_message::ws_message(const std::string& str)
{
    util::split(str, payload);
    if(!payload.empty()){
        try{
            this->event = static_cast<wsEvent>(std::stoi(payload.front()));
            payload.erase(payload.begin());
        }catch(...){
            event = wsEvent_invalid;
            payload.clear();
        }


    }

}
ws_message::ws_message()
{

}
// Report a failure
void fail(beast::error_code ec, char const* what){
    std::cerr << what << ": " << ec.message() << "\n";
}

ws_session::ws_session(tcp::socket&& socket, ssl::context& ctx, WebsocketServer* websocketServer) : ws_(std::move(socket), ctx){
    this->websocketServer_m = websocketServer;
    dnSubscriptions.insert("6");
}
ws_session::~ws_session()
{

}
void ws_session::run(){
    // Set the timeout.
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    ws_.next_layer().async_handshake( ssl::stream_base::server, beast::bind_front_handler( &ws_session::on_handshake, shared_from_this() ) );
}

void ws_session::on_handshake(beast::error_code ec){
    if(ec)
        return fail(ec, "handshake");

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option( websocket::stream_base::timeout::suggested( beast::role_type::server) );

    // Set a decorator to change the Server of the handshake
    ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-server-async-ssl");
            }
    ));

    // Accept the websocket handshake
    ws_.async_accept( beast::bind_front_handler( &ws_session::on_accept, shared_from_this() ) );
}
void ws_session::on_accept(beast::error_code ec){
    if(ec)
        return fail(ec, "accept");

    websocketServer_m->addSession(shared_from_this());
    // Read a message
    asyncReading();
}

void ws_session::asyncReading(){
    // Read a message into our buffer
    ws_.async_read( buffer_in, beast::bind_front_handler( &ws_session::after_read, shared_from_this() ) );
}

void ws_session::after_read( beast::error_code ec, std::size_t bytes_transferred){
    boost::ignore_unused(bytes_transferred);

    // This indicates that the ws_session was closed
    if(ec == websocket::error::closed)
        return;
    if(ec)
        fail(ec, "read");
    if(ws_.got_text()){
        ws_message msg(beast::buffers_to_string(buffer_in.cdata()));
        if(msg.event == wsEvent_authentification || authenticated_m){
            dispatch(msg, shared_from_this());
        }

    }
    buffer_in.consume(bytes_transferred);
    asyncReading(); //loop
}

void ws_session::on_write( beast::error_code ec, std::size_t bytes_transferred){
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail(ec, "write");
    outQueue.pop();

    if(! outQueue.empty()){
        ws_.async_write(net::buffer(outQueue.front()->to_Str()), beast::bind_front_handler( &ws_session::on_write, shared_from_this()));
    }
}

void ws_session::sendFiltered(std::shared_ptr<ws_message>& msg){
    net::post(ws_.get_executor(), beast::bind_front_handler(&ws_session::on_send, shared_from_this(), msg, true));
}
void ws_session::send(std::shared_ptr<ws_message>& msg){
    net::post(ws_.get_executor(), beast::bind_front_handler(&ws_session::on_send, shared_from_this(), msg, false));
}
void ws_session::on_send(std::shared_ptr<ws_message> msg, bool filterEn){
    if(authenticated_m){
        //filter here for actual view
        if(filterEn){
            if(msg->event == wsEvent_dataNodeChange){
                if(checkDataNodeSubscription(msg->payload[0])){
                    outQueue.push(msg);
                } else {
                    return;
                }
            }else if(msg->event == wsEvent_paramNodeChange){
                if(checkParamNodeSubscription(msg->payload[0])){
                    outQueue.push(msg);
                } else{
                    return;
                }
            }else{
                outQueue.push(msg);
            }
        }else{
            outQueue.push(msg);
        }

        if(outQueue.size() > 1){
            return;
        }
        ws_.async_write(net::buffer(outQueue.front()->to_Str()), beast::bind_front_handler( &ws_session::on_write, shared_from_this()));
    }
}

void ws_session::dispatch(const ws_message& msg, std::shared_ptr<ws_session> ws_session_)
{
    websocketServer_m->ws_dispatch(msg, ws_session_);
}
bool ws_session::checkDataNodeSubscription(const std::string& sqlId)
{
    return util::includes(dnSubscriptions, sqlId);
}
bool ws_session::checkParamNodeSubscription(const std::string& sqlId)
{
    return util::includes(paramSubscriptions, sqlId);
}
void ws_session::setAuthenticated()
{
    authenticated_m = true;
}
//------------------------------------------------------------------------------

listener::listener( net::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, WebsocketServer* websocketServer) : ioc_(ioc), ctx_(ctx), acceptor_(net::make_strand(ioc)){
    beast::error_code ec;

    this->websocketServer_m = websocketServer;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec){
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if(ec){
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec){
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen( net::socket_base::max_listen_connections, ec );
    if(ec)
    {
        fail(ec, "listen");
        return;
    }
}

// Start accepting incoming connections
void listener::run(){
    do_accept();
}

void listener::do_accept(){
    // The new connection gets its own strand
    acceptor_.async_accept( net::make_strand(ioc_), beast::bind_front_handler( &listener::on_accept, shared_from_this() ) );
}

void listener::on_accept(beast::error_code ec, tcp::socket socket){
    if(ec){
        fail(ec, "accept");
    } else {
        // Create the ws_session and run it
        std::shared_ptr<ws_session> ws_session_ = std::make_shared<ws_session>(std::move(socket), ctx_, this->websocketServer_m);
        ws_session_->run();

    }

    // Accept another connection
    do_accept();
}


//------------------------------------------------------------------------------



WebsocketServer::WebsocketServer(){
    // This holds the self-signed certificate used by the server
    load_server_certificate(ctx);

    // Create and launch a listening port
    listener_m = std::make_shared<listener>(ioc, ctx, tcp::endpoint{address, port}, this);
    listener_m->run();

    // Run the I/O service on the requested number of threads

    ioThreads.reserve(threadCnt);
    for(int i= 0; i< threadCnt; i++)
    {
        ioThreads.emplace_back(
        [this]
        {
            this->ioc.run();
        });

    }


}
WebsocketServer::~WebsocketServer(){
    ioc.stop();
    int watchdogCnt = 20;
    while(!ioc.stopped()){
        util::ConsoleOut() << "waiting for Websocket IO Service to stop";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        watchdogCnt--;
        if(watchdogCnt <= 0){
            util::ConsoleOut() << "waiting for Websocket IO Service to stop FAILED, Watchdog forced destrucktion of WebsocketServer Object";
            break;
        }
    }

}
void WebsocketServer::addSession(std::shared_ptr<ws_session> ws_session_p)
{
    ws_sessionsMutex_m.lock();
    std::lock_guard<std::mutex> lg(ws_sessionsMutex_m, std::adopt_lock);
    std::weak_ptr<ws_session> wp = ws_session_p;
    ws_sessions_m.push_back(wp);
}
void WebsocketServer::removeDeletedSessions() // evt unnötig (wenn geprüft wird ob der pointer noch gültig ist kann die ws_session auch hier gelöscht werden (über itterator (is ja gelockt)))
{
    ws_sessionsMutex_m.lock();
    std::lock_guard<std::mutex> lg(ws_sessionsMutex_m, std::adopt_lock);
    ws_sessions_m.remove_if(
                [](auto const& a){
                    std::shared_ptr<ws_session> b = a.lock();
                        if(b){
                            return false;
                        }else{
                            return true;
                        }
                         });
}
void WebsocketServer::publishtoAllSessions(const ws_message &msg)
{

    std::shared_ptr<ws_message> msg_sp = std::make_shared<ws_message>(msg);

    ws_sessionsMutex_m.lock();
    std::lock_guard<std::mutex> lg(ws_sessionsMutex_m, std::adopt_lock);

    for(auto&& ws_session_wp : ws_sessions_m){
        if(std::shared_ptr<ws_session> ws_session_sp = ws_session_wp.lock()){
            ws_session_sp->sendFiltered(msg_sp);
        }
    }
}
