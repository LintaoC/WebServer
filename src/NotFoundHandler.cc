#include "NotFoundHandler.h"
#include <string>

NotFoundHandler::NotFoundHandler() {
}

NotFoundHandler::~NotFoundHandler() {
}

boost::beast::http::response<boost::beast::http::string_body>
NotFoundHandler::handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) {
    boost::beast::http::response<boost::beast::http::string_body> res;

    // Set the status of the response
    res.result(boost::beast::http::status::not_found);

    // Set the content type of the response
    res.set(boost::beast::http::field::content_type, "text/html");

    // Set the body of the response
    res.body() = "<html><body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body></html>";

    // Prepare the response to be sent
    res.prepare_payload();

    return res;
}