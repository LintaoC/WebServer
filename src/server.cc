#include "../include/server.h"
#include "../include/session.h"
#include "../include/config_parser.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
using namespace boost::placeholders;

// Constructor for the 'server' class
server::server(boost::asio::io_service &io_service, short port)
    : io_service_(io_service),                              // Initialize the io_service_ member with the provided io_service
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) // Initialize the acceptor to listen on the given port with IPv4
{
    std::cout << "Server started on port " << port << std::endl;
    start_accept(); // Begin accepting connections
}

// start accepting incoming connections
void server::start_accept()
{
    session *new_session = new session(io_service_); // Create a new session for the incoming connection
    // Asynchronously accept connections. When a connection is accepted, handle it with 'handle_accept'
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
}

// handle the completion of an asynchronous accept operation
void server::handle_accept(session *new_session, const boost::system::error_code &error)
{
    if (!error)
    {
        new_session->start(); // Start the session if there was no error
    }
    else
    {
        delete new_session; // If there was an error, delete the session object to free resources
    }

    start_accept(); // Call start_accept again to accept next incoming connection
}

