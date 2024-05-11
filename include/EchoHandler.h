#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include "RequestHandler.h"

// EchoHandler class that echoes back the request
class EchoHandler : public RequestHandler {
public:
    EchoHandler();
    ~EchoHandler();
    // Overridden method to handle requests
    boost::beast::http::response<boost::beast::http::string_body>
    handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) override;
};

#endif // ECHO_HANDLER_H