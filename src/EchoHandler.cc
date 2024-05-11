#include "EchoHandler.h"


EchoHandler::EchoHandler(){
}

EchoHandler::~EchoHandler() {
}


// Implementation of handle_request for EchoHandler
boost::beast::http::response<boost::beast::http::string_body>
EchoHandler::handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) {
    boost::beast::http::response<boost::beast::http::string_body> res;

    // Set the status code to 200 OK
    res.result(boost::beast::http::status::ok);

    // Create a string stream to build the full request text
    std::stringstream ss;

    // Serialize the request base (method, target, version, headers)
    ss << req.method_string() << " " << req.target() << " HTTP/" << (req.version() / 10) << "." << (req.version() % 10) << "\r\n";
    for (const auto& field : req.base()) {
        ss << field.name_string() << ": " << field.value() << "\r\n";
    }

    // Append the request body
    ss << "\r\n" << req.body();

    // Set the response body to the entire formatted request
    res.set(boost::beast::http::field::content_type, "text/plain");
    res.body() = ss.str();

    // Prepare the response
    res.prepare_payload();

    return res;
}