#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include "../include/session.h"
#include "../include/RequestHandlerFactory.h"
#include "../include/RequestHandler.h"

using namespace testing;
using namespace boost::asio;
using namespace boost::beast::http;
using ip::tcp;

class EchoRequestHandler : public RequestHandler {
public:
    response<string_body> handle_request(const request<string_body>& req) override {
        response<string_body> res{status::ok, req.version()};
        res.set(field::server, "Test");
        res.set(field::content_type, "text/plain");
        res.body() = "Echo";
        res.prepare_payload();
        return res;
    }
};

class EchoRequestHandlerFactory : public RequestHandlerFactory {
public:
    EchoRequestHandlerFactory() : RequestHandlerFactory("EchoHandler", {{"root", "/echo"}}) {}
    RequestHandler* buildRequestHandler() const override {
        return new EchoRequestHandler();
    }
};

class SessionTest : public Test {
protected:
    io_service io_service_;
    std::map<std::string, RequestHandlerFactory*> routes_;
    std::shared_ptr<session> session_;
    std::shared_ptr<tcp::acceptor> acceptor_;

    void SetUp() override {
        auto echo_factory = new EchoRequestHandlerFactory();
        routes_["/echo"] = echo_factory;
        session_ = std::make_shared<session>(io_service_, &routes_);
        acceptor_ = std::make_shared<tcp::acceptor>(io_service_);

        // Initialize logging
        boost::log::add_common_attributes();
        boost::log::add_console_log(std::cout, boost::log::keywords::format = "%TimeStamp% [%Severity%]: %Message%");
    }

    void TearDown() override {
        for (auto& route : routes_) {
            delete route.second;
        }
    }
};

TEST_F(SessionTest, Start) {
    auto endpoint = tcp::endpoint(tcp::v4(), 8080);
    acceptor_->open(endpoint.protocol());
    acceptor_->set_option(tcp::acceptor::reuse_address(true));
    acceptor_->bind(endpoint);
    acceptor_->listen();

    // Set a timer to stop the io_service after 5 seconds to prevent it from running forever
    boost::asio::steady_timer timer(io_service_, boost::asio::chrono::seconds(5));
    timer.async_wait([&](const boost::system::error_code& /*e*/) {
        io_service_.stop();
    });

    acceptor_->async_accept(session_->socket(), [&](const boost::system::error_code& ec) {
        if (!ec) {
            session_->start();
        }
        io_service_.stop();
    });

    // Simulate a connection by creating a client and connecting to the server
    std::thread client_thread([&]() {
        io_service client_io_service;
        tcp::resolver resolver(client_io_service);
        auto endpoints = resolver.resolve("127.0.0.1", "8080");
        tcp::socket socket(client_io_service);
        boost::asio::connect(socket, endpoints);
        std::string request_str = "GET /echo HTTP/1.1\r\nHost: localhost\r\n\r\n";
        boost::asio::write(socket, boost::asio::buffer(request_str));
        socket.close();
    });

    io_service_.run();
    client_thread.join();
    // EXPECT_TRUE(session_->socket().is_open());
}

TEST_F(SessionTest, HandleReadSuccess) {
    auto echo_factory = new EchoRequestHandlerFactory();
    routes_["/echo"] = echo_factory;

    std::string request_str = "GET /echo HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::copy(request_str.begin(), request_str.end(), session_->data_);

    boost::system::error_code ec;
    size_t bytes_transferred = request_str.size();
    session_->handle_read(ec, bytes_transferred);

    // Test if the response has been written back to the socket (mock)
    // EXPECT_TRUE(session_->socket().is_open());
}

// TEST_F(SessionTest, HandleReadError) {
//     boost::system::error_code ec = boost::asio::error::connection_reset;
//     size_t bytes_transferred = 0;

//     session_->handle_read(ec, bytes_transferred);
//     // EXPECT_FALSE(session_->socket().is_open());
// }

TEST_F(SessionTest, GetRequestHandlerFactory) {
    auto factory = session_->getRequestHandlerFactory("/echo", &routes_);
    // EXPECT_EQ(factory, routes_["/echo"]);
}

TEST_F(SessionTest, HandleAcceptError) {
    auto endpoint = tcp::endpoint(tcp::v4(), 8080);
    acceptor_->open(endpoint.protocol());
    acceptor_->set_option(tcp::acceptor::reuse_address(true));
    acceptor_->bind(endpoint);
    acceptor_->listen();

    // Set a timer to stop the io_service after 5 seconds to prevent it from running forever
    boost::asio::steady_timer timer(io_service_, boost::asio::chrono::seconds(5));
    timer.async_wait([&](const boost::system::error_code& /*e*/) {
        io_service_.stop();
    });

    acceptor_->async_accept(session_->socket(), [&](const boost::system::error_code& ec) {
        session_->handle_read(ec, 0);
        io_service_.stop();
    });

    io_service_.run();
    // EXPECT_FALSE(session_->socket().is_open());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Initialize Boost.Log
    boost::log::add_common_attributes();
    boost::log::add_console_log(std::cout, boost::log::keywords::format = "%TimeStamp% [%Severity%]: %Message%");

    return RUN_ALL_TESTS();
}
