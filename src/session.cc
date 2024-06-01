#include "session.h"
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
#include <memory>

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
                            boost::bind(&session::handle_read, shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred) {
    std::cerr << "get into the handle_read" << std::endl;
    try {
        auto self = shared_from_this();
        std::cerr << "after shared_from_this() function" << std::endl;
        if (!ec) {
            std::cerr << "get into the !ec" << std::endl;
            BOOST_LOG_TRIVIAL(info) << "Successfully read";
            boost::beast::multi_buffer buffer;
            buffer.commit(boost::asio::buffer_copy(buffer.prepare(bytes_transferred), boost::asio::buffer(data_, bytes_transferred)));
            boost::beast::http::request_parser<boost::beast::http::string_body> parser;
            parser.eager(true);
            boost::system::error_code parse_ec;
            parser.put(buffer.data(), parse_ec);
            if (parse_ec) {
                std::cerr << "get into the parse_ec" << std::endl;
                BOOST_LOG_TRIVIAL(error) << "Parsing failed: " << parse_ec.message();
                send_bad_request_response();
                return;
            }
            if (parser.is_done()) {
                std::cerr << "get into the parser done" << std::endl;
                auto req = parser.release();
                BOOST_LOG_TRIVIAL(info) << "Request parsed with method: " << req.method() << ", target: " << req.target()
                                        << ", version: " << req.version() << ", body: " << req.body();
                if (!validate_request(req)) {
                    std::cerr << "get into non validate request" << std::endl;
                    send_bad_request_response();
                    return;
                }
                std::cerr << "it is validate request" << std::endl;
                RequestHandlerFactory* factory = getRequestHandlerFactory(std::string(req.target()), routes_);
                std::cerr << "after getting requestHandlerfactory" << std::endl;
                if (!factory) {
                    BOOST_LOG_TRIVIAL(error) << "No matching handler found";
                    send_bad_request_response();
                    return;
                }
                RequestHandler* handler = factory->buildRequestHandler();
                std::cerr << "after creating handler" << std::endl;
                boost::beast::http::response<boost::beast::http::string_body> response = handler->handle_request(req);
                delete handler;

            BOOST_LOG_TRIVIAL(info) << "[ResponseMetrics] "
                                    << "ResponseCode=" << response.result_int() << " "
                                    << "RequestPath=" << std::string(req.target()) << " "
                                    << "RequestIP=" << socket_.remote_endpoint().address().to_string() << ":" << socket_.remote_endpoint().port() << " "
                                    << "Handler=" << factory->getHandlerType() << " ";
            // log_response_metrics(req, response, factory->getHandlerType(), socket_.remote_endpoint());
                auto sp = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(std::move(response));
                std::cerr << "start write" << std::endl;
                boost::beast::http::async_write(socket_, *sp,
                                                [this, sp, self](const boost::system::error_code& ec, std::size_t length) {
                                                    if (!ec) {
                                                        BOOST_LOG_TRIVIAL(info) << "Response sent successfully";
                                                    } else {
                                                        BOOST_LOG_TRIVIAL(error) << "Error sending response: " << ec.message();
                                                    }
                                                    socket_.close();
                                                });
                std::cerr << "after write" << std::endl;
            } else {
                socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                        boost::bind(&session::handle_read, shared_from_this(),
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred));
            }
        } else {
            std::cerr << "there is a ec" << std::endl;
            BOOST_LOG_TRIVIAL(error) << "Error on receive: " << ec.message();
            socket_.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in handle_read: " << e.what() << std::endl;
        throw;
    }
}

bool session::validate_request(const boost::beast::http::request<boost::beast::http::string_body>& req) {
    if (req.method() == boost::beast::http::verb::unknown) {
        BOOST_LOG_TRIVIAL(error) << "Invalid HTTP method";
        return false;
    }
    if (req.find(boost::beast::http::field::host) == req.end()) {
        BOOST_LOG_TRIVIAL(error) << "Missing Host header";
        return false;
    }

    int host_count = 0;
    for (auto it = req.begin(); it != req.end(); ++it) {
        if (it->name() == boost::beast::http::field::host) {
            host_count++;
        }
        if (host_count > 1) {
            BOOST_LOG_TRIVIAL(error) << "Duplicate Host headers";
            return false;
        }

        std::string header_name = it->name_string().to_string();
        std::string header_value = it->value().to_string();
        if (!std::all_of(header_name.begin(), header_name.end(), [](unsigned char c) { return std::isalnum(c) || c == '-' || c == '_'; }) ||
            !std::all_of(header_value.begin(), header_value.end(), [](unsigned char c) { return std::isprint(c); })) {
            BOOST_LOG_TRIVIAL(error) << "Invalid characters in header";
            return false;
        }
    }

    std::string uri = req.target().to_string();
    if (!std::all_of(uri.begin(), uri.end(), [](unsigned char c) { return ::isascii(c); })) {
        BOOST_LOG_TRIVIAL(error) << "Non-ASCII characters in URI";
        return false;
    }

    if (req.method() == boost::beast::http::verb::post && req.find(boost::beast::http::field::content_length) == req.end()) {
        BOOST_LOG_TRIVIAL(error) << "Missing Content-Length header in POST request";
        return false;
    }
    return true;
}

void session::send_bad_request_response() {
    auto self = shared_from_this();
    auto sp = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
    sp->result(boost::beast::http::status::bad_request);
    sp->body() = "Bad Request\n";
    sp->set(boost::beast::http::field::content_type, "text/plain");
    sp->prepare_payload();

    boost::beast::http::async_write(socket_, *sp,
                                    [this, sp, self](const boost::system::error_code& ec, std::size_t length) {
                                        if (!ec) {
                                            BOOST_LOG_TRIVIAL(info) << "400 Bad Request response sent successfully";
                                        } else {
                                            BOOST_LOG_TRIVIAL(error) << "Error sending 400 response: " << ec.message();
                                        }
                                        socket_.close();
                                    });
}
RequestHandlerFactory* session::getRequestHandlerFactory(const std::string& path, std::map<std::string, RequestHandlerFactory*>* routes) {
    BOOST_LOG_TRIVIAL(info) << "getRequestHandlerFactory received a path of " << path;
    RequestHandlerFactory* longestMatchFactory = nullptr;
    size_t longestMatchLength = 0;
    std::string relativePath = "";

    for (const auto& route : *routes) {
        const std::string& prefix = route.first;
        if (path.length() >= prefix.length() && path.compare(0, prefix.length(), prefix) == 0) {
            if (path.length() == prefix.length() || path[prefix.length()] == '/') {
                if (prefix.length() > longestMatchLength) {
                    longestMatchFactory = route.second;
                    longestMatchLength = prefix.length();
                    relativePath = path.substr(prefix.length());
                }
            }
        }
    }

    if ((longestMatchFactory && longestMatchLength > 1) || path.length() == 1) {
        if (!relativePath.empty() && relativePath[0] != '/') {
            relativePath = '/' + relativePath;
        }
        longestMatchFactory->setRelativePath(relativePath);
        return longestMatchFactory;
    }

    return new RequestHandlerFactory("", {});
}
