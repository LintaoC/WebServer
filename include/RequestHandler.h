#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <boost/beast/http.hpp>

// Abstract base class for all request handlers
class RequestHandler {
public:
    virtual ~RequestHandler() {}
    // Pure virtual function to handle the request and produce a response
    virtual boost::beast::http::response<boost::beast::http::string_body>
    handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) = 0;
};

#endif // REQUEST_HANDLER_H