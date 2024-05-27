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
#include <boost/asio/signal_set.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include "../include/server.h"
#include "../include/config_parser.h"
#include "../include/SessionFactory.h"
#include "RequestHandlerFactory.h"
#include <map>
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
void init_logging() {
    namespace logging = boost::log;
    namespace keywords = logging::keywords;
    namespace sinks = logging::sinks;
    namespace expr = logging::expressions;
    namespace attrs = logging::attributes;

    // Add Thread ID to the logging attributes
    logging::core::get()->add_global_attribute("ThreadID", attrs::current_thread_id());

    // Create a console logger
    logging::add_console_log(
        std::cout,
        keywords::format = (
            expr::stream
            << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
            << " [Thread " << expr::attr<attrs::current_thread_id::value_type>("ThreadID") << "]"
            << ": <" << logging::trivial::severity << "> " << expr::smessage
        )
    );

    // Create a file logger
    logging::add_file_log(
        keywords::file_name = "../log/server_log_%Y-%m-%d_%H-%M-%S.log",
        keywords::auto_flush = true,
        keywords::rotation_size = 10 * 1024 * 1024,  // file rotation at 10 MB
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::format = (
            expr::stream
            << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
            << " [Thread " << expr::attr<attrs::current_thread_id::value_type>("ThreadID") << "]"
            << ": <" << logging::trivial::severity << "> " << expr::smessage
        )
    );

    // Add common attributes
    logging::add_common_attributes();
}

void worker_thread(boost::asio::io_service& io_service) {
    io_service.run();
}

int main(int argc, char* argv[]) {
    init_logging();
    BOOST_LOG_TRIVIAL(trace) << "Initializing server logging";
    BOOST_LOG_TRIVIAL(info) << "Server starting up";

    if (argc != 2) {
        std::cerr << "Usage: server <config_file>\n";
        BOOST_LOG_TRIVIAL(error) << "Usage: server <config_file>";
        return 1;
    }

    NginxConfigParser config_parser;
    NginxConfig config;
    if (!config_parser.Parse(argv[1], &config)) {
        std::cerr << "Failed to parse config file\n";
        BOOST_LOG_TRIVIAL(fatal) << "Failed to parse config file";
        return 1;
    }

    int port = config.GetPort();
    if (port == -1) {
        std::cerr << "Port number not found in the configuration file.\n";
        BOOST_LOG_TRIVIAL(fatal) << "Port number not found in the configuration file.";
        return 1;
    }

    std::map<std::string, RequestHandlerFactory*>* routes = config.getPathMap();

    boost::asio::io_service io_service;
    SessionFactory factory;

    try {
        server s(io_service, port, &factory, routes);
        BOOST_LOG_TRIVIAL(debug) << "Server initialized successfully on port: " << port;

        // Create a pool of threads to run all of the io_services.
        std::vector<std::thread> threads;
        std::size_t thread_count = std::thread::hardware_concurrency();
        if (thread_count < 4) {
            thread_count = 4;  // Default to 4 if less than 4
        }

        // Launch threads to process the io_service
        for (std::size_t i = 0; i < thread_count; ++i) {
            threads.emplace_back(std::thread(worker_thread, std::ref(io_service)));
        }

        // Handle signals for graceful shutdown
        boost::asio::signal_set signals(io_service, SIGINT, SIGTERM);
        signals.async_wait(
                [&](const boost::system::error_code& error, int signal_number) {
                    BOOST_LOG_TRIVIAL(info) << "Server terminating by signal: " << signal_number;
                    io_service.stop();
                }
        );

        // Wait for all threads to complete
        for (auto& t : threads) {
            t.join();
        }

        BOOST_LOG_TRIVIAL(info) << "Server shutdown successfully";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        BOOST_LOG_TRIVIAL(fatal) << "Server initialization failed: " << e.what();
        return 1;
    }

    return 0;
}
