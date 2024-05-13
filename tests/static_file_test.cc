#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/asio.hpp>
#include "StaticFileHandler.h"

using boost::asio::ip::tcp;
using namespace testing;

class MockSocket : public tcp::socket
{
public:
    explicit MockSocket(boost::asio::io_service &service) : tcp::socket(service) {}

    MOCK_METHOD4(async_write, void(const boost::asio::mutable_buffers_1 &, boost::function<void(const boost::system::error_code &, std::size_t)>));
};

class StaticFileHandlerTest : public Test
{
protected:
    boost::asio::io_service io_service;
    std::unique_ptr<MockSocket> mock_socket;
    std::unique_ptr<StaticFileHandler> handler;

    void SetUp() override
    {
        mock_socket = std::make_unique<MockSocket>(io_service);
        handler = std::make_unique<StaticFileHandler>(*mock_socket, "/test/path");
    }
};

TEST_F(StaticFileHandlerTest, HandleRequest_FileExists)
{
    std::string request = "GET /test.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
    // EXPECT_CALL(*mock_socket, async_write(_, _)).Times(AtLeast(1)); // Expect that async_write is called at least once
    handler->handleRequest(request);
    // Additional checks can be added here to verify correct file content is sent
}

TEST_F(StaticFileHandlerTest, HandleRequest_FileNotFound)
{
    std::string request = "GET /nonexistent.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
    // EXPECT_CALL(*mock_socket, async_write(_, _)).Times(1); // Expect that async_write is called exactly once for the error response
    handler->handleRequest(request);
    // You can add additional expectations here to check if the correct error response is being sent
}
