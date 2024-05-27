#include "SleepRequestHandler.h"
#include <thread>
#include <chrono>

// Default constructor
SleepRequestHandler::SleepRequestHandler() {
}

// Destructor
SleepRequestHandler::~SleepRequestHandler() {
}

boost::beast::http::response<boost::beast::http::string_body> SleepRequestHandler::handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) {
    std::this_thread::sleep_for(std::chrono::seconds(5));  // Block for 5 seconds

    boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::ok, req.version()};
    res.set(boost::beast::http::field::server, "TestServer");
    res.set(boost::beast::http::field::content_type, "text/plain");
    res.body() = "Sleep Request Handler Response";
    res.prepare_payload();
    return res;
}
