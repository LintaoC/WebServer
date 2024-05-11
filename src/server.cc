#include "../include/server.h"
#include "../include/session.h"
#include "../include/config_parser.h"
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/bind/bind.hpp>
#include "RequestHandlerFactory.h"
#include <map>
using namespace boost::placeholders;

// Constructor for the 'server' class
server::server(boost::asio::io_service& io_service, short port, ISessionFactory* factory, std::map<std::string, RequestHandlerFactory*>* routes)
    : io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), factory_(factory), routes_(routes){
    start_accept();  // Begin accepting connections
}

// Start accepting incoming connections
void server::start_accept() {
    session* new_session = factory_->create(io_service_, routes_);  // Use factory to create a session
    BOOST_LOG_TRIVIAL(debug) << "Ready to accept new connection";
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
}

// Handle the completion of an asynchronous accept operation
void server::handle_accept(session* new_session, const boost::system::error_code& error) {
    BOOST_LOG_TRIVIAL(info) << "Accepted new connection";
    if (!error) {
        new_session->start();  // Start the session if there was no error
        BOOST_LOG_TRIVIAL(info) << "Session started successfully";
    } else {
        BOOST_LOG_TRIVIAL(error) << "Error accepting connection: " << error.message();
        delete new_session;  // If there was an error, delete the session object to free resources
    }
    start_accept();  // Call start_accept again to accept next incoming connection
}

