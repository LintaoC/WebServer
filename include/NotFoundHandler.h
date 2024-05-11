#ifndef NOT_FOUND_HANDLER_H
#define NOT_FOUND_HANDLER_H

#include "RequestHandler.h"
#include <boost/beast/http.hpp>

class NotFoundHandler : public RequestHandler {
public:
    // Constructor
    NotFoundHandler();

    // Destructor
    virtual ~NotFoundHandler();

    // Overridden handle_request method from RequestHandler
    virtual boost::beast::http::response<boost::beast::http::string_body>
    handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) override;
};

#endif // NOT_FOUND_HANDLER_H
