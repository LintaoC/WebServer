#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "EchoHandler.h"
#include <boost/asio.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;
using boost::asio::ip::tcp;
using ::testing::_;
using ::testing::Invoke;
using namespace testing;
class MockSocket : public tcp::socket {
public:
    MockSocket(io_service& service) : tcp::socket(service) {}

    MOCK_METHOD0(close, void());
    MOCK_METHOD2(async_write, void(const boost::asio::const_buffer&, std::function<void(const boost::system::error_code&, std::size_t)>));
};

class EchoHandlerTest : public ::testing::Test {
protected:
    io_service service;
    MockSocket* socket;
    EchoHandler* handler;

    EchoHandlerTest() : socket(new MockSocket(service)) {}

    void SetUp() override {
        handler = new EchoHandler(*socket);
    }

    void TearDown() override {
        delete handler;  // Ensure handler is deleted before the socket
        delete socket;
    }
};

TEST_F(EchoHandlerTest, HandleRequestTest) {
    std::string request_data = "GET / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 15\r\n\r\npartial_body";

    //Set up expectations and behavior for the async_write method
    // EXPECT_CALL(*socket, async_write(_, _))
    //     .WillOnce(Invoke([this](const boost::asio::const_buffer& buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
    //         // Check that some buffer is written and the size is reasonable
    //         ASSERT_GT(boost::asio::buffer_size(buffer), 0);
    //         handler(boost::system::error_code(), boost::asio::buffer_size(buffer)); // Simulate completion
    //     }));

    // EXPECT_CALL(*socket, close()).Times(AtLeast(1));

    handler->handleRequest(request_data);

    // Uncomment the line below to ensure the io_service processes the completion handler
    service.run();
}