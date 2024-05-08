#include "StaticFileHandler.h"
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using namespace boost::placeholders;
//1.correct input
//2.file not found
StaticFileHandler::StaticFileHandler(tcp::socket& socket, const std::string& root_path)
    : socket_(socket), root_path_(root_path) {}

StaticFileHandler::~StaticFileHandler() {
    if (file_.is_open()) {
        file_.close(); // Ensure the file is closed when the handler is destroyed
    }
}

void StaticFileHandler::handleRequest(const std::string& request_data) {
    std::istringstream request_stream(request_data);
    std::string request_line;
    std::getline(request_stream, request_line);
    std::istringstream line_stream(request_line);
    std::string method;
    line_stream >> method >> path_;
    serveFile(root_path_);
}

void StaticFileHandler::serveFile(const std::string& path) {
    file_.open(path, std::ios::binary);
    if (!file_.is_open()) {
        BOOST_LOG_TRIVIAL(error) << "Failed to open file: " << path;
        sendErrorResponse(404, "File Not Found");
        return;
    }
    BOOST_LOG_TRIVIAL(info) << "Serving file: " << path;
    file_.seekg(0, std::ios::end);
    std::size_t file_size = file_.tellg();
    file_.seekg(0);

    std::ostringstream headers;
    headers << "HTTP/1.1 200 OK\r\n";
    headers << "Content-Type: " << determineContentType(path) << "\r\n";
    headers << "Content-Length: " << file_size << "\r\n";
    headers << "Connection: close\r\n\r\n";
    boost::asio::async_write(socket_, boost::asio::buffer(headers.str()),
                             boost::bind(&StaticFileHandler::sendFileChunk, this));
}

void StaticFileHandler::sendFileChunk() {
    std::cerr<<"test sendfile chunk, go into the sendfile chunk"<<std::endl;
    buffer_.resize(1024 * 64);
    file_.read(buffer_.data(), buffer_.size());
    std::size_t bytes_read = file_.gcount();
    std::cerr<<" sendfile chunk,after file_count"<<std::endl;
    if (bytes_read > 0) {
        
        boost::asio::async_write(socket_, boost::asio::buffer(buffer_.data(), bytes_read),
                                 boost::bind(&StaticFileHandler::sendFileChunk, this));
    } else {
        std::cerr<<" what is bytes_read"<<bytes_read<<std::endl;
        std::cerr<<" after reading bytes_read but not 0"<<std::endl;
        BOOST_LOG_TRIVIAL(info) << "File transmission completed for: " << root_path_;
        socket_.close();  // Close the socket after the last chunk has been sent
        delete this;
        
    }
    std::cerr<<"successfully read bytes read"<<std::endl;
}
void StaticFileHandler::sendErrorResponse(int status_code, const std::string& message) {
    BOOST_LOG_TRIVIAL(warning) << "Sending error response: " << status_code << " " << message;
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " " << message << "\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << message.length() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << "<html><body><h1>" << status_code << " " << message << "</h1></body></html>";

    std::string response_str = response.str();
    boost::asio::async_write(
            socket_, boost::asio::buffer(response_str),
            boost::bind(&StaticFileHandler::handleWriteCompletion, this, boost::asio::placeholders::error)
    );
}

void StaticFileHandler::handleWriteCompletion(const boost::system::error_code& ec) {
    if (!ec) {
        BOOST_LOG_TRIVIAL(info) << "Error response sent and connection closing.";
        socket_.close();  // Close the socket after the response has been fully written
    }
    delete this;  // Delete the handler object
}

std::string StaticFileHandler::determineContentType(const std::string& path) {
    if (endsWith(path, ".html")) return "text/html";
    if (endsWith(path, ".jpg") || endsWith(path, ".jpeg")) return "image/jpeg";
    if (endsWith(path, ".png")) return "image/png";
    if (endsWith(path, ".txt")) return "text/plain";
    if (endsWith(path, ".zip")) return "application/zip";
    return "application/octet-stream"; // Default MIME type
}

bool StaticFileHandler::endsWith(const std::string& value, const std::string& ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}