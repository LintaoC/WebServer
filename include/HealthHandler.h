// HealthHandler.h
#ifndef HEALTH_HANDLER_H
#define HEALTH_HANDLER_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include <string>
#include "RequestHandler.h"

class HealthHandler : public RequestHandler {
public:
    HealthHandler();
    ~HealthHandler();

    boost::beast::http::response<boost::beast::http::string_body> handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req);
};

#endif // HEALTH_HANDLER_H
