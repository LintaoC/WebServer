#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include "../include/server.h"
#include "session.h"
#include "RequestHandlerFactory.h"

using namespace testing;
using namespace boost::asio;
using ip::tcp;

// Mock class for Session
class MockSession : public session {
public:
    MockSession(boost::asio::io_service& io_service, std::map<std::string, RequestHandlerFactory*>* routes)
        : session(io_service, routes), socket_(io_service) {}

    MOCK_METHOD0(start, void());
    MOCK_METHOD0(get_socket, tcp::socket&());

    tcp::socket& socket() { return socket_; }

private:
    tcp::socket socket_;
};

// Mock class for ISessionFactory
class MockSessionFactory : public ISessionFactory {
public:
    MOCK_METHOD2(create, session*(boost::asio::io_service&, std::map<std::string, RequestHandlerFactory*>*));
};

// ServerTest class
class ServerTest : public Test {
protected:
    io_service io_service_;
    MockSessionFactory mock_factory_;
    std::map<std::string, RequestHandlerFactory*> routes_;
    std::shared_ptr<server> server_;
    deadline_timer timer_;

    ServerTest() : timer_(io_service_) {}

    void SetUp() override {
        // Initialize logging
        boost::log::add_common_attributes();
        boost::log::add_console_log(std::cout, boost::log::keywords::format = "%TimeStamp% [%Severity%]: %Message%");
        
        server_ = std::make_shared<server>(io_service_, 8080, &mock_factory_, &routes_);
    }

    void StartTimeout() {
        timer_.expires_from_now(boost::posix_time::seconds(5)); // Set a timeout of 5 seconds
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec) {
                io_service_.stop();
                BOOST_LOG_TRIVIAL(error) << "Test timeout reached, stopping io_service.";
            }
        });
    }
};

TEST_F(ServerTest, StartAccept) {
    BOOST_LOG_TRIVIAL(info) << "Test StartAccept begins";
    auto mock_session = std::make_shared<MockSession>(io_service_, &routes_);
    EXPECT_CALL(mock_factory_, create(_, _)).WillOnce(Return(mock_session.get()));
    // EXPECT_CALL(*mock_session, get_socket()).WillOnce(ReturnRef(mock_session->socket()));

    StartTimeout(); // Start the timeout timer
    server_->start_accept();
    io_service_.run();
    BOOST_LOG_TRIVIAL(info) << "Test StartAccept ends";
}

TEST_F(ServerTest, HandleAcceptSuccess) {
    BOOST_LOG_TRIVIAL(info) << "Test HandleAcceptSuccess begins";
    auto mock_session = std::make_shared<MockSession>(io_service_, &routes_);
    boost::system::error_code ec;  // No error

    EXPECT_CALL(*mock_session, start());

    server_->handle_accept(mock_session.get(), ec);

    // Ensure start_accept is called again
    auto new_mock_session = std::make_shared<MockSession>(io_service_, &routes_);
    // EXPECT_CALL(mock_factory_, create(_, _)).WillOnce(Return(new_mock_session.get()));
    // EXPECT_CALL(*new_mock_session, get_socket()).WillOnce(ReturnRef(new_mock_session->socket()));

    StartTimeout(); // Start the timeout timer
    server_->start_accept();

    io_service_.run();
    BOOST_LOG_TRIVIAL(info) << "Test HandleAcceptSuccess ends";
}

// TEST_F(ServerTest, HandleAcceptError) {
//     BOOST_LOG_TRIVIAL(info) << "Test HandleAcceptError begins";
//     auto mock_session = std::make_shared<MockSession>(io_service_, &routes_);
//     boost::system::error_code ec = boost::asio::error::connection_refused;  // Simulate an error

//     // EXPECT_CALL(*mock_session, start()).Times(0);

//     server_->handle_accept(mock_session.get(), ec);

//     // Ensure start_accept is called again
//     auto new_mock_session = std::make_shared<MockSession>(io_service_, &routes_);
//     // EXPECT_CALL(mock_factory_, create(_, _)).WillOnce(Return(new_mock_session.get()));
//     // EXPECT_CALL(*new_mock_session, get_socket()).WillOnce(ReturnRef(new_mock_session->socket()));

//     StartTimeout(); // Start the timeout timer
//     server_->start_accept();

//     io_service_.run();
//     BOOST_LOG_TRIVIAL(info) << "Test HandleAcceptError ends";
// }

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);

    // Initialize Boost.Log
    boost::log::add_common_attributes();
    boost::log::add_console_log(std::cout, boost::log::keywords::format = "%TimeStamp% [%Severity%]: %Message%");
    
    return RUN_ALL_TESTS();
}