#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "session.h"
#include "config_parser.h"

using namespace testing;
using namespace boost::asio;
using ip::tcp;

class MockSocket : public tcp::socket {
public:
    explicit MockSocket(io_service& service) : tcp::socket(service) {}
    MOCK_METHOD(tcp::endpoint, remote_endpoint, (), (const));
};

class MockNginxConfig : public NginxConfig {
public:
    //explicit MockSocket(io_service& service) : tcp::socket(service) {}
    MOCK_METHOD(std::string, GetHandlerType, (const std::string&),(const));
    MOCK_METHOD(std::string, GetFilePath, (const std::string&), (const));
};

// Mocking the session class
class MockSession : public session {
public:
    MockSession(boost::asio::io_service& io_service, MockNginxConfig& config)
        : session(io_service, config) {}

};

class SessionTest : public Test {
protected:
    io_service io_service_;
    MockNginxConfig mock_config_;
    std::shared_ptr<MockSession> mock_session_;
    void SetUp() override
    {
    // Create a MockSession object and assign it to mock_session_
        mock_session_ = std::make_shared<MockSession>(io_service_, mock_config_);

    }
};

TEST_F(SessionTest, HandleCompleteRequest) {

    

    // Set up the complete request
    std::string complete_request = "GET /echo/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";

    // Expect GetHandlerType to be called with "/echo/a.txt" and return "echo"
    //EXPECT_CALL(mock_config_, GetHandlerType("/echo/a.txt")).WillOnce(Return("echo"));

    // Simulate reading data into session
    boost::system::error_code ec;  // No error simulated
    memcpy(mock_session_->data_, complete_request.data(), complete_request.size());
    mock_session_->handle_read(ec, complete_request.size()); // This will trigger the start() method

    // Ensure all async operations complete
    io_service_.run(); 
}
