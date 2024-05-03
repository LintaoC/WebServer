#include "EchoHandler.h"
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;

EchoHandler::EchoHandler(tcp::socket& socket) : socket_(socket) {}

void EchoHandler::handleRequest(const std::string& request_data) {
    BOOST_LOG_TRIVIAL(debug) << "Echo request handling started";
    std::ostringstream response_stream;
    response_stream << "HTTP/1.1 200 OK\r\n"
                    << "Content-Type: text/plain\r\n"
                    << "Content-Length: " << request_data.length() << "\r\n"
                    << "Date: " << get_date() << "\r\n"
                    << "\r\n"
                    << request_data;

    boost::asio::async_write(socket_,
                             boost::asio::buffer(response_stream.str()),
                             boost::bind(&EchoHandler::handleWriteCompletion, this));
}

void EchoHandler::handleWriteCompletion() {
    BOOST_LOG_TRIVIAL(info) << "Echo response sent and connection closing.";
    socket_.close();  // Close the socket after the response has been fully written
    delete this;  // Delete the handler object
}

std::string EchoHandler::get_date() {
    std::time_t now = std::time(nullptr);
    struct tm tm = *std::gmtime(&now);
    char buf[30];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm);
    return std::string(buf);
}