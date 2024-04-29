#include "../include/session.h"
#include <boost/bind/bind.hpp>
#include <sstream>
#include <ctime>
#include <iostream>
#include "../include/EchoHandler.h"
#include "../include/StaticFileHandler.h"
#include "../include/RequestHandler.h"
using namespace boost::placeholders;

session::session(boost::asio::io_service &io_service)
    : socket_(io_service) {}

tcp::socket &session::socket()
{
    return socket_;
}

void session::start()
{
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            boost::bind(&session::handle_read, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

void session::parse_request() {
    std::istringstream request_stream(request_data_);
    std::string request_line;
    std::getline(request_stream, request_line); // Read the first line which contains the request details
    std::istringstream line_stream(request_line);
    line_stream >> method_; // GET, POST, etc.
    line_stream >> path_; // "/echo", "/static/filename", etc.
}

void session::handle_read(const boost::system::error_code &error, size_t bytes_transferred)
{
    if (!error)
    {
        request_data_.append(data_, bytes_transferred);
        auto headers_end = request_data_.find("\r\n\r\n");
        if (headers_end != std::string::npos)
        {
            headers_end += 4; // Account for the length of "\r\n\r\n"
            size_t content_length = get_content_length(request_data_);
            if (request_data_.size() >= headers_end + content_length)
            {
                // We have the full request, including headers and body
                parse_request();
                std::ostringstream response_stream;
                RequestHandler* handler;
                if(path_.compare(0, 7, "/static") == 0)
                {
                    handler = new StaticFileHandler(socket_, "../files");  // Dynamically allocates memory for a StaticFileHandler object
                }
                else{
                    handler = new EchoHandler(socket_);
                }
                handler->handleRequest(request_data_);
            }
            else
            {
                // Continue reading more data
                socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                        boost::bind(&session::handle_read, this,
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred));
            }
        }
        else
        {
            // Continue reading more data
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                    boost::bind(&session::handle_read, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
    }
    else
    {
        std::cerr << "Read error: " << error.message() << "\n";
        delete this; // Properly handle the deletion of this session
    }
}

size_t session::get_content_length(const std::string &request)
{
    std::size_t pos = request.find("Content-Length:");
    if (pos != std::string::npos)
    {
        std::size_t start = request.find_first_of("0123456789", pos);
        std::size_t end = request.find("\r\n", start);
        return std::stoi(request.substr(start, end - start));
    }
    return 0;
}

std::string session::get_date()
{
    std::time_t now = std::time(0);
    struct tm tm = *std::gmtime(&now);
    char buf[30];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &tm);
    return std::string(buf);
}

void session::handle_write(const boost::system::error_code &error)
{

    delete this; // Close and delete session
    
}

