#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <string>

using boost::asio::ip::tcp;

class session {
public:
    explicit session(boost::asio::io_service& io_service);
    tcp::socket& socket();
    virtual void start();

//private:
    virtual void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);
    virtual void send_response();
    size_t get_content_length(const std::string& request);  // Helper function to parse Content-Length
    std::string get_date();  // Helper function to get the current date in HTTP date format

    tcp::socket socket_;
    enum { max_length = 8192 };
    char data_[max_length];
    std::string request_data_;
};


#endif