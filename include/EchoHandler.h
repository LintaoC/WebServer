#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include "RequestHandler.h"
#include <boost/asio.hpp>
#include <sstream>
#include <ctime>

class EchoHandler : public RequestHandler {
public:
    EchoHandler(boost::asio::ip::tcp::socket& socket);
    virtual void handleRequest(const std::string& request_data) override;


    virtual void handleWriteCompletion();
    boost::asio::ip::tcp::socket& socket_;
    std::string get_date();
};

#endif 