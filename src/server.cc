#include "../include/server.h"
#include "../include/session.h"
#include "../include/config_parser.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
using namespace boost::placeholders;

// Constructor for the 'server' class
server::server(boost::asio::io_service& io_service, short port, ISessionFactory* factory)
    : io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), factory_(factory) {
    start_accept();  // Begin accepting connections
}

// Start accepting incoming connections
void server::start_accept() {
    session* new_session = factory_->create(io_service_);  // Use factory to create a session
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
}

// Handle the completion of an asynchronous accept operation
void server::handle_accept(session* new_session, const boost::system::error_code& error) {
    if (!error) {
        new_session->start();  // Start the session if there was no error
    } else {
        delete new_session;  // If there was an error, delete the session object to free resources
    }
    start_accept();  // Call start_accept again to accept next incoming connection
}

