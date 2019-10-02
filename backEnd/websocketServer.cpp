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


// Report a failure
void fail(beast::error_code ec, char const* what){
    std::cerr << what << ": " << ec.message() << "\n";
}

session::session(tcp::socket&& socket, ssl::context& ctx) : ws_(std::move(socket), ctx){

}

void session::run(){
    // Set the timeout.
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    ws_.next_layer().async_handshake( ssl::stream_base::server, beast::bind_front_handler( &session::on_handshake, shared_from_this() ) );
}

void session::on_handshake(beast::error_code ec){
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
    ws_.async_accept( beast::bind_front_handler( &session::on_accept, shared_from_this() ) );
}
void session::on_accept(beast::error_code ec){
    if(ec)
        return fail(ec, "accept");

    // Read a message
    do_read();
}

void session::do_read(){
    // Read a message into our buffer
    ws_.async_read( buffer_, beast::bind_front_handler( &session::on_read, shared_from_this() ) );
}

void session::on_read( beast::error_code ec, std::size_t bytes_transferred){
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if(ec == websocket::error::closed)
        return;
    if(ec)
        fail(ec, "read");

    // Echo the message
    ws_.text(ws_.got_text());
    ws_.async_write( buffer_.data(), beast::bind_front_handler( &session::on_write, shared_from_this() ) );
}

void session::on_write( beast::error_code ec, std::size_t bytes_transferred){
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail(ec, "write");

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Do another read
    do_read();
}


//------------------------------------------------------------------------------

listener::listener( net::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint) : ioc_(ioc), ctx_(ctx), acceptor_(net::make_strand(ioc)){
    beast::error_code ec;

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
        // Create the session and run it
        std::make_shared<session>(std::move(socket), ctx_)->run();
    }

    // Accept another connection
    do_accept();
}


//------------------------------------------------------------------------------



WebsocketServer::WebsocketServer(){


    // This holds the self-signed certificate used by the server
    load_server_certificate(ctx);

    // Create and launch a listening port
    std::make_shared<listener>(ioc, ctx, tcp::endpoint{address, port})->run();

    // Run the I/O service on the requested number of threads

    ioThreads.reserve(threads);
    ioThreads.emplace_back(
    [this]
    {
        this->ioc.run();
    });


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




