#include "HealthHandler.h"

HealthHandler::HealthHandler() {
}

HealthHandler::~HealthHandler() {
}

// Implementation of handle_request for HealthHandler
boost::beast::http::response<boost::beast::http::string_body>
HealthHandler::handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) {
    boost::beast::http::response<boost::beast::http::string_body> res;

    // Set the status code to 200 OK
    res.result(boost::beast::http::status::ok);

    // Set the response body to "OK"
    res.body() = "OK";
    res.set(boost::beast::http::field::content_type, "text/plain");

    // Prepare the response
    res.prepare_payload();

    return res;
}