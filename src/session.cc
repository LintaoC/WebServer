#include "../include/session.h"
#include <boost/bind/bind.hpp>
#include <sstream>
#include <ctime>
#include <iostream>
#include "../include/EchoHandler.h"
#include "../include/StaticFileHandler.h"
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
                std::ostringstream response_stream;
                if(path_.compare(0, 7, "/static") == 0)
                {
                    EchoHandler handler(socket_);
                    handler.handleRequest(request_data_);

                }
                else{
                    StaticFileHandler handler(socket_, "../files");
                    handler.handleRequest(request_data_);
                }
                //handler.handleRequest(request_data_, response_stream);
                //send_response(response_stream);
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

// void session::send_response(std::ostringstream& response_stream)
// {
//     boost::asio::async_write(socket_,
//                                 boost::asio::buffer(response_stream.str()),
//                                 boost::bind(&session::handle_write, this,
//                                             boost::asio::placeholders::error));
// }

// void session::serve_file() {
//     file_.open("../files" + path_.substr(7), std::ios::binary);
//     if (!file_.is_open()) {
//         send_error_response(404, "File Not Found");
//         return;
//     }

//     // Prepare HTTP headers
//     std::ostringstream response_stream;
//     file_.seekg(0, std::ios::end);
//     std::size_t file_size = file_.tellg();
//     file_.seekg(0);

//     response_stream << "HTTP/1.1 200 OK\r\n";
//     response_stream << "Content-Type: " << determine_content_type() << "\r\n";
//     response_stream << "Content-Length: " << file_size << "\r\n";
//     response_stream << "Connection: close\r\n\r\n";  // Ensure clients know to close after completion

//     // Asynchronously send the HTTP header
//     boost::asio::async_write(socket_, boost::asio::buffer(response_stream.str()),
//                              [this](const boost::system::error_code& error, std::size_t) {
//                                  if (!error) {
//                                      send_file_chunk();  // Begin sending file content in chunks
//                                  } else {
//                                      std::cerr << "Error sending headers: " << error.message() << std::endl;
//                                  }
//                              });
// }

// void session::send_file_chunk() {
//     buffer_.resize(1024 * 64);  // Smaller chunks, e.g., 64KB
//     file_.read(buffer_.data(), buffer_.size());
//     std::size_t bytes_read = file_.gcount();

//     if (bytes_read > 0) {
//         boost::asio::async_write(socket_, boost::asio::buffer(buffer_.data(), bytes_read),
//                                  [this](const boost::system::error_code& error, std::size_t) {
//                                      if (!error) {
//                                          send_file_chunk();  // Continue sending next chunk
//                                      } else {
//                                          std::cerr << "Error sending file data: " << error.message() << std::endl;
//                                      }
//                                  });
//     } else {
//         socket_.close();  // Close the socket after the last chunk has been sent
//     }
// }

// bool ends_with(const std::string& value, const std::string& ending) {
//     if (ending.size() > value.size()) return false;
//     return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
// }

// std::string session::determine_content_type() {
//     // Ensure that path_ is a valid and correctly initialized member variable
//     if (ends_with(path_, ".html")) return "text/html";
//     if (ends_with(path_, ".jpg") || ends_with(path_, ".jpeg")) return "image/jpeg";
//     if (ends_with(path_, ".png")) return "image/png";
//     if (ends_with(path_, ".txt")) return "text/plain";
//     if (ends_with(path_, ".zip")) return "application/zip";
//     return "application/octet-stream"; // Default MIME type
// }

// void session::send_error_response(int status_code, const std::string& message) {
//     std::ostringstream response_stream;
//     response_stream << "HTTP/1.1 " << status_code << " " << message << "\r\n";
//     response_stream << "Content-Type: text/html\r\n";
//     response_stream << "Content-Length: " << message.length() << "\r\n";
//     response_stream << "Connection: close\r\n";
//     response_stream << "\r\n";
//     response_stream << "<html><body><h1>" << status_code << " " << message << "</h1></body></html>";

//     auto response = response_stream.str();
//     boost::asio::async_write(socket_, boost::asio::buffer(response),
//                              [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
//                                  if (!error) {
//                                      std::cout << "Error response sent successfully." << std::endl;
//                                  } else {
//                                      std::cerr << "Failed to send error response: " << error.message() << std::endl;
//                                  }
//                              });
// }


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

