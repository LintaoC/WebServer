#include "StaticFileHandler.h"
#include <boost/bind/bind.hpp>

using boost::asio::ip::tcp;
using namespace boost::placeholders;

StaticFileHandler::StaticFileHandler(tcp::socket& socket, const std::string& root_path)
    : socket_(socket), root_path_(root_path) {}

StaticFileHandler::~StaticFileHandler() {}

void StaticFileHandler::handleRequest(const std::string& request_data) {
    std::istringstream request_stream(request_data);
    std::string request_line;
    std::getline(request_stream, request_line);
    std::istringstream line_stream(request_line);
    std::string method;
    line_stream >> method >> path_;
    serveFile(root_path_ + path_);
}

void StaticFileHandler::serveFile(const std::string& path) {
    file_.open(path, std::ios::binary);
    if (!file_.is_open()) {
        sendErrorResponse(404, "File Not Found");
        return;
    }

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
    buffer_.resize(1024 * 64);  // Smaller chunks
    file_.read(buffer_.data(), buffer_.size());
    std::size_t bytes_read = file_.gcount();

    if (bytes_read > 0) {
        boost::asio::async_write(socket_, boost::asio::buffer(buffer_.data(), bytes_read),
                                 boost::bind(&StaticFileHandler::sendFileChunk, this));
    } else {
        socket_.close();
    }
}

void StaticFileHandler::sendErrorResponse(int status_code, const std::string& message) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " " << message << "\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << message.length() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << "<html><body><h1>" << status_code << " " << message << "</h1></body></html>";

    boost::asio::async_write(socket_, boost::asio::buffer(response.str()),
                             [](const boost::system::error_code& error, std::size_t) {});
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