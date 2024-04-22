//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include "../include/server.h"
#include "../include/config_parser.h"
#include "../include/SessionFactory.h" 
// int main(int argc, char *argv[])
// {

//   try
//   {
//     if (argc != 2)
//     {
//       std::cerr << "Usage: async_tcp_echo_server <port>\n";
//       return 1;
//     }

//     boost::asio::io_service io_service; // Create an io_service object which provides I/O services, like sockets

//     using namespace std;                 // For atoi.
//     server s(io_service, atoi(argv[1])); // Create an instance of the server class, initializing it with the I/O service and the port number to listen on

//     io_service.run(); // Run the io_service object to perform asynchronous network operations
//   }
//   // if ant exception is thrown, catch it and print an error message
//   catch (std::exception &e)
//   {
//     std::cerr << "Exception: " << e.what() << "\n";
//   }

//   return 0;
// }

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: server <config_file>\n";
        return 1;
    }

    NginxConfigParser config_parser;
    NginxConfig config;
    if (!config_parser.Parse(argv[1], &config)) {
        std::cerr << "Failed to parse config file\n";
        return 1;
    }

    int port = config.GetPort();
    if (port == -1) {
        std::cerr << "Port number not found in the configuration file.\n";
        return 1;
    }

    boost::asio::io_service io_service;
    SessionFactory factory;
    try {
        server s(io_service, port,&factory);
        io_service.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
