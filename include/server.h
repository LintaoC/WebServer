#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include "session.h"

using boost::asio::ip::tcp;

class server
{
public:
    server(boost::asio::io_service &io_service, short port);
    // now its taking a file
    //  server(boost::asio::io_service& io_service, const std::string &config_file);

private:
    void start_accept();
    void handle_accept(session *new_session, const boost::system::error_code &error);

    boost::asio::io_service &io_service_;
    tcp::acceptor acceptor_;
};

#endif