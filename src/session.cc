#include "../include/session.h"
#include <boost/bind/bind.hpp>
#include <sstream>
#include <ctime>
#include <iostream>
#include <boost/log/trivial.hpp>
#include "../include/EchoHandler.h"
#include "../include/StaticFileHandler.h"
#include "../include/RequestHandler.h"
#include "../include/config_parser.h"
using namespace boost::placeholders;

session::session(boost::asio::io_service &io_service, NginxConfig config)
    : socket_(io_service), config_(config) {}

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

void session::parse_request() {
    BOOST_LOG_TRIVIAL(trace) << "Parsing request: " << request_data_;
    std::istringstream request_stream(request_data_);
    std::string request_line;
    std::getline(request_stream, request_line); // Read the first line which contains the request details
    std::istringstream line_stream(request_line);
    line_stream >> method_; // GET, POST, etc.
    line_stream >> path_; // "/echo", "/static/filename", etc.
    BOOST_LOG_TRIVIAL(info) << "Received request for " << path_ << " using method " << method_;
}

std::string GetRemainingPath(const std::string& path) {
    if (path.empty() || path[0] != '/') {
        return ""; // Return an empty string for empty or invalid paths.
    }
    // Find the position of the second slash.
    size_t second_slash_pos = path.find('/', 1);
    if (second_slash_pos == std::string::npos) {
        return ""; // Return an empty string if no second slash is found.
    }

    return path.substr(second_slash_pos); // Return the remaining substring after the first component.
}

void session::handle_read(const boost::system::error_code &error, size_t bytes_transferred)
{
    if (!error)
    {
        request_data_.append(data_, bytes_transferred);
        //BOOST_LOG_TRIVIAL(info) << "Data read successfully: " << bytes_transferred << " bytes";
        auto headers_end = request_data_.find("\r\n\r\n");
        if (headers_end != std::string::npos)
        {
            headers_end += 4; // Account for the length of "\r\n\r\n"
            size_t content_length = get_content_length(request_data_);
            if (request_data_.size() >= headers_end + content_length)
            {
                // We have the full request, including headers and body
                parse_request();
                BOOST_LOG_TRIVIAL(info) << "Request parsed: " << method_ << " " << path_;
                std::ostringstream response_stream;
                std::cerr<<"The path  is"<<path_<<std::endl;
                std::string type =config_.GetHandlerType(path_);
                std::cerr<<"The type is"<<type<<std::endl;
                RequestHandler* handler;
                if( type == "static")
                {
                     handler = new StaticFileHandler(socket_, config_.GetFilePath(path_)+
                                                                            GetRemainingPath(path_));  // Dynamically allocates memory for a StaticFileHandler object
                }
                else if (type == "echo"){
                    handler = new EchoHandler(socket_);
                }
                if(type!="")
                {
                    handler->handleRequest(request_data_);
                } 
                
          
                
            }
            else
            {
                // Continue reading more data
                socket_.async_read_some(boost::asio::buffer(data_, max_length),boost::bind(&session::handle_read, this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
            }
        }
        else
        {
            // Continue reading more data
            socket_.async_read_some(boost::asio::buffer(data_, max_length),boost::bind(&session::handle_read, this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        }
    }
    else
    {
        std::cerr << "Read error: " << error.message() << "\n";
        BOOST_LOG_TRIVIAL(fatal) << "Read error: " << error.message();
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

