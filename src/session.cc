#include "../include/session.h"
#include <boost/bind/bind.hpp>
#include <sstream>
#include <ctime>
#include <iostream>
#include <boost/log/trivial.hpp>
#include "../include/RequestHandler.h"
#include "../include/config_parser.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
using namespace boost::placeholders;

session::session(boost::asio::io_service &io_service, std::map<std::string, RequestHandlerFactory*>* routes)
    : socket_(io_service), routes_(routes) {}

tcp::socket &session::socket()
{
    return socket_;
}

void session::start()
{
    auto endpoint = socket_.remote_endpoint();
    BOOST_LOG_TRIVIAL(info) << "Connection from: " << endpoint.address().to_string() << ":" << endpoint.port();
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            boost::bind(&session::handle_read, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
                                     
}

void session::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred) {
    if (!ec) {
        BOOST_LOG_TRIVIAL(info)<<"Successfully read";
        // Assuming 'data_' is a suitable buffer where data from socket is read into.
        // Ensure 'data_' is declared, probably as a member of 'session'.

        // Prepare the multi_buffer for input
        boost::beast::multi_buffer buffer;

        // Write data into the multi_buffer
        // Here, we convert 'data_' which is probably a char array into a sequence that can be appended to the buffer
        std::size_t size = bytes_transferred; // The actual size of the data read
        buffer.commit(boost::asio::buffer_copy(buffer.prepare(size), boost::asio::buffer(data_, size)));

        // Create and use the parser
        boost::beast::http::request_parser<boost::beast::http::string_body> parser;
        parser.eager(true); // Optional: parse headers eagerly

        // Parse the HTTP request from the buffer
        boost::system::error_code parse_ec;
        parser.put(buffer.data(), parse_ec);
        if(parse_ec) {
            BOOST_LOG_TRIVIAL(error) << "Parsing failed: " << parse_ec.message() << std::endl;
            return;
        }

        if (parser.is_done()) {
            // Complete parsing, extract the request
            auto req = parser.release();
            BOOST_LOG_TRIVIAL(info)<<"request parsed with method: "<<req.method()<<",   target: "<<req.target()<<
            ",    version: "<<req.version()<<",     base: "<<req.base()<<",     body: "<<req.body();
            RequestHandlerFactory* factory = getRequestHandlerFactory(std::string(req.target()), routes_);
            BOOST_LOG_TRIVIAL(info)<<"Factory class created with a factory of "<<factory->getHandlerType();
            RequestHandler* handler= factory->buildRequestHandler();
            boost::beast::http::response<boost::beast::http::string_body> response = handler->handle_request(req);
            BOOST_LOG_TRIVIAL(info) << "Response generated with a base of: " << response.base();
            delete handler;
            // Serialize and write the response asynchronously
            auto sp = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(std::move(response));
            boost::beast::http::async_write(socket_, *sp,
                                            [this, sp](const boost::system::error_code& ec, std::size_t length) {
                                                if (!ec) {
                                                    BOOST_LOG_TRIVIAL(info) << "Response sent successfully";
                                                } else {
                                                    BOOST_LOG_TRIVIAL(error) << "Error sending response: " << ec.message();
                                                }
                                                delete this; // Safely delete the session object after operation completes
                                            });
        } else {
            // If the request is not fully parsed, continue reading
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                    boost::bind(&session::handle_read, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
    } else {
        BOOST_LOG_TRIVIAL(error) << "Error on receive: " << ec.message() << "\n";
        socket_.close();
        delete this;
    }
}

RequestHandlerFactory* session::getRequestHandlerFactory(const std::string& path, std::map<std::string, RequestHandlerFactory*>* routes) {
    BOOST_LOG_TRIVIAL(info)<<"getRequestHandlerFactory received a path of "<<path;

    RequestHandlerFactory* longestMatchFactory = nullptr;
    size_t longestMatchLength = 0;
    std::string relativePath;  // To store the relative path to be set

    // Iterate over all routes to find the longest matching prefix
    for (const auto& route : *routes) {
        const std::string& prefix = route.first;
        // Check for matching prefix that also ensures the match respects path boundaries
        if (path.length() >= prefix.length() && path.compare(0, prefix.length(), prefix) == 0) {
            if (path.length() == prefix.length() || path[prefix.length()] == '/') {
                if (prefix.length() > longestMatchLength) {
                    longestMatchFactory = route.second;
                    longestMatchLength = prefix.length();
                    // Extract the relative path part
                    relativePath = path.substr(prefix.length());
                }
            }
        }
    }

    // If a match is found, set the relative path and return the factory
    if (longestMatchFactory) {
        if (!relativePath.empty() && relativePath[0] != '/') {
            relativePath = '/' + relativePath;  // Ensure leading '/' for relative path
        }
        longestMatchFactory->setRelativePath(relativePath);
        return longestMatchFactory;
    }

    // Return nullptr if no matching factory is found
    return nullptr;
}




