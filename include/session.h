#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <string>
#include <fstream>
#include "../include/config_parser.h"

using boost::asio::ip::tcp;

class session {
public:
    explicit session(boost::asio::io_service& io_service, NginxConfig config);
    tcp::socket& socket();
    virtual void start();

//private:
    virtual void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    virtual void handle_write(const boost::system::error_code &error);
    //virtual void serve_file();
    //virtual void send_response();
    virtual void parse_request();
    size_t get_content_length(const std::string& request);  // Helper function to parse Content-Length
    std::string get_date();  // Helper function to get the current date in HTTP date format
    //std::string determine_content_type();
    //virtual void send_error_response(int status_code, const std::string& message);
    //virtual void send_file_chunk();

    tcp::socket socket_;
    enum { max_length = 8192 };
    char data_[max_length];
    std::string request_data_;
    std::string method_;
    std::string path_;
    std::ifstream file_;
    std::vector<char> buffer_;
    NginxConfig config_;
};


#endif