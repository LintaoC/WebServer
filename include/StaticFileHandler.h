#ifndef STATIC_FILE_HANDLER_H
#define STATIC_FILE_HANDLER_H

#include "RequestHandler.h"
#include <fstream>
#include <boost/asio.hpp>

class StaticFileHandler : public RequestHandler {
public:
    StaticFileHandler(boost::asio::ip::tcp::socket& socket, const std::string& root_path);
    virtual ~StaticFileHandler() override;
    void handleRequest(const std::string& request_data) override;

private:
    boost::asio::ip::tcp::socket& socket_;
    std::string root_path_;
    std::ifstream file_;
    std::string path_;
    std::vector<char> buffer_;

    void serveFile(const std::string& path);
    void sendFileChunk();
    void sendErrorResponse(int status_code, const std::string& message);
    std::string determineContentType(const std::string& path);
    bool endsWith(const std::string& value, const std::string& ending);
    void handleWriteCompletion(const boost::system::error_code& ec);
};

#endif // STATIC_FILE_HANDLER_H