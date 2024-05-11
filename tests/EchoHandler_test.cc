//#include <gtest/gtest.h>
//#include <gmock/gmock.h>
//#include "EchoHandler.h"
//#include <boost/asio.hpp>
//
//using namespace boost::asio;
//using namespace boost::asio::ip;
//using boost::asio::ip::tcp;
//using ::testing::_;
//using ::testing::Invoke;
//using namespace testing;
//
//class MockSocket : public tcp::socket {
//public:
//    MockSocket(io_service& service) : tcp::socket(service) {}
//
//    MOCK_METHOD0(close, void());
//    MOCK_METHOD2(async_write, void(const boost::asio::const_buffer&, std::function<void(const boost::system::error_code&, std::size_t)>));
//};
//
//class MockEchoHandler : public EchoHandler {
//public:
//    MockEchoHandler(tcp::socket& socket) : EchoHandler(socket) {}
//    MOCK_METHOD0(handleWriteCompletion, void());
//};
//
//class EchoHandlerTest : public ::testing::Test {
//protected:
//    io_service service;
//    MockSocket* socket;
//    MockEchoHandler* handler;
//
//    EchoHandlerTest() : socket(new MockSocket(service)) {}
//
//    void SetUp() override {
//        handler = new MockEchoHandler(*socket);
//    }
//
//    void TearDown() override {
//        delete handler;  // Ensure handler is deleted before the socket
//        delete socket;
//    }
//};
//
//TEST_F(EchoHandlerTest, HandleRequestTest) {
//std::string request_data = "GET / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 15\r\n\r\npartial_body";
//
//// Set up the expectation for handleWriteCompletion being called.
//EXPECT_CALL(*handler, handleWriteCompletion()).Times(1);
//
//
//// Trigger the request handling.
//handler->handleRequest(request_data);
//
//// Ensure the io_service processes the completion handler.
//service.run();
//}