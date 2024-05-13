// StaticFileHandler.h
#ifndef STATIC_FILE_HANDLER_H
#define STATIC_FILE_HANDLER_H

#include "RequestHandler.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <string>
#include <fstream>

class StaticFileHandler : public RequestHandler {
public:
    StaticFileHandler(const std::string& root_path, const std::string& relative_path);
    ~StaticFileHandler();

    // Implementing the pure virtual function from RequestHandler
    boost::beast::http::response<boost::beast::http::string_body>
    handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) override;

private:
    std::string root_path_;
    std::string relative_path_;
    std::ifstream file_;

    std::string determineContentType(const std::string& path);
    bool endsWith(const std::string& value, const std::string& ending);
};

#endif // STATIC_FILE_HANDLER_H
