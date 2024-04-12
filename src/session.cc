#include "session.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
using namespace boost::placeholders;

// Constructor for the 'session' class
session::session(boost::asio::io_service &io_service)
    : socket_(io_service) // Initialize the socket with the provided io_service
{
}

// get a reference to the socket associated with this session
tcp::socket &session::socket()
{
    return socket_;
}

// start the session by reading from the socket
void session::start()
{
    // Begin asynchronous read operation on the socket to read data into the buffer 'data_'
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            boost::bind(&session::handle_read, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

// handle the completion of a read operation
void session::handle_read(const boost::system::error_code &error, size_t bytes_transferred)
{
    // If there was no error, write the data back to the client
    if (!error)
    {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(data_, bytes_transferred),
                                 boost::bind(&session::handle_write, this,
                                             boost::asio::placeholders::error));
        }
    else
    {
        delete this;
    }
}

// handle the completion of a write operation
void session::handle_write(const boost::system::error_code &error)
{
    std::cout << "session received: complete";
    // Check if the write operation completed successfully
    if (!error)
    {
        // If successful, start another read operation to continue receiving data from the client
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        delete this;
    }
}
