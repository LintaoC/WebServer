#include "session.h"
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
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
    auto self(shared_from_this());
    auto endpoint = socket_.remote_endpoint();
    BOOST_LOG_TRIVIAL(info) << "Connection from: " << endpoint.address().to_string() << ":" << endpoint.port();
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            boost::bind(&session::handle_read, self,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred) {
    if (!ec) {
        BOOST_LOG_TRIVIAL(info) << "Successfully read";

        boost::beast::multi_buffer buffer;
        buffer.commit(boost::asio::buffer_copy(buffer.prepare(bytes_transferred), boost::asio::buffer(data_, bytes_transferred)));

        boost::beast::http::request_parser<boost::beast::http::string_body> parser;
        parser.eager(true);
        boost::system::error_code parse_ec;
        parser.put(buffer.data(), parse_ec);

        if (parse_ec) {
            BOOST_LOG_TRIVIAL(error) << "Parsing failed: " << parse_ec.message();
            return;
        }

        if (parser.is_done()) {
            auto req = parser.release();
            BOOST_LOG_TRIVIAL(info) << "request parsed with method: " << req.method() << ", target: " << req.target() << ", version: " << req.version() << ", base: " << req.base() << ", body: " << req.body();

            RequestHandlerFactory* factory = getRequestHandlerFactory(std::string(req.target()), routes_);
            BOOST_LOG_TRIVIAL(info) << "Factory class created with a factory of " << factory->getHandlerType();
            RequestHandler* handler = factory->buildRequestHandler();
            boost::beast::http::response<boost::beast::http::string_body> response = handler->handle_request(req);
            BOOST_LOG_TRIVIAL(info) << "Response generated with a base of: " << response.base();
            delete handler;

            auto sp = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(std::move(response));
            auto self(shared_from_this());
            boost::beast::http::async_write(socket_, *sp,
                                            [self, sp](const boost::system::error_code& ec, std::size_t length) {
                                                self->handle_write(ec, length);
                                            });
        } else {
            auto self(shared_from_this());
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                    boost::bind(&session::handle_read, self,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
    } else {
        BOOST_LOG_TRIVIAL(error) << "Error on receive: " << ec.message();
        socket_.close();
    }
}

void session::handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred) {
    if (!ec) {
        BOOST_LOG_TRIVIAL(info) << "Response sent successfully";
    } else {
        BOOST_LOG_TRIVIAL(error) << "Error sending response: " << ec.message();
    }
    boost::system::error_code ignored_ec;
    socket_.shutdown(tcp::socket::shutdown_both, ignored_ec);
    socket_.close();
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





