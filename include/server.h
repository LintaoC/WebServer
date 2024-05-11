#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include "session.h"
#include "ISessionFactory.h"
#include "../include/config_parser.h"
#include "RequestHandlerFactory.h"
#include <map>

using boost::asio::ip::tcp;

class server
{
public:
    server(boost::asio::io_service &io_service, short port,ISessionFactory* factory, std::map<std::string, RequestHandlerFactory*>* routes);
    // now its taking a file
    //  server(boost::asio::io_service& io_service, const std::string &config_file);
    void start_accept();
    void handle_accept(session *new_session, const boost::system::error_code &error);

    boost::asio::io_service &io_service_;
    tcp::acceptor acceptor_;
    ISessionFactory* factory_;
    std::map<std::string, RequestHandlerFactory*>* routes_;
};

#endif
