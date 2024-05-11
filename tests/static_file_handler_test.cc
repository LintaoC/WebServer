//#include <gtest/gtest.h>
//#include <gmock/gmock.h>
//#include <boost/asio.hpp>
//#include <boost/system/error_code.hpp> // For boost::system::error_code
//#include "StaticFileHandler.h"
//
//using namespace testing;
//using boost::asio::ip::tcp;
//
//class MockSocket : public tcp::socket {
//public:
//    explicit MockSocket(boost::asio::io_service& io_service) : tcp::socket(io_service) {}
//
//    MOCK_METHOD(void, async_write, (const boost::asio::const_buffer& buffers, std::function<void(const boost::system::error_code&, std::size_t bytes_transferred)> handler), ());
//};
//
//class StaticFileHandlerTest : public ::testing::Test {
//protected:
//    boost::asio::io_service io_service;
//    std::unique_ptr<MockSocket> mock_socket;
//    std::unique_ptr<StaticFileHandler> handler;
//
//    void SetUp() override {
//        mock_socket = std::make_unique<MockSocket>(io_service);
//        handler = std::make_unique<StaticFileHandler>(*mock_socket, "/valid/path");
//    }
//};
//
//TEST_F(StaticFileHandlerTest, HandleRequest_FileExists) {
//    std::string request = "GET /static/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
//
//    // EXPECT_CALL(*mock_socket, async_write(_, _))
//    //     .Times(AtLeast(1));
//
//    handler->handleRequest(request);
//}
//TEST_F(StaticFileHandlerTest, HandleRequest_serverfile) {
//    std::string request = "GET /static/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
//
//    // EXPECT_CALL(*mock_socket, async_write(_, _))
//    //     .Times(AtLeast(1));
//
//    handler->serveFile(request);
//}
//TEST_F(StaticFileHandlerTest, HandleRequest_sendChunk) {
//    int status_code = 404;
//    std::string message = "File Not Found";
//
//    // Prepare the expected response string
//    std::ostringstream expected_response;
//    expected_response << "HTTP/1.1 " << status_code << " " << message << "\r\n"
//                      << "Content-Type: text/html\r\n"
//                      << "Content-Length: " << message.length() << "\r\n"
//                      << "Connection: close\r\n\r\n"
//                      << "<html><body><h1>" << status_code << " " << message << "</h1></body></html>";
//
//    // Expect the socket's async_write to be called with the correct data
//    // EXPECT_CALL(*mock_socket, async_write(_, _))
//    //     .Times(1)
//    //     .WillOnce(Invoke([&](const boost::asio::const_buffer& buffers, auto handler) {
//    //         // Convert buffer to string to check content
//    //         const char* data = boost::asio::buffer_cast<const char*>(buffers);
//    //         std::string content(data, boost::asio::buffer_size(buffers));
//    //         EXPECT_EQ(content, expected_response.str());
//    //         // Simulate successful write operation
//    //         handler(boost::system::error_code(), content.size());
//    //     }));
//
//    // Call the method under test
//    handler->sendErrorResponse(status_code, message);
//}
//TEST_F(StaticFileHandlerTest, HandleRequest_determineContentType) {
//    std::string request = "GET /static/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
//
//    // EXPECT_CALL(*mock_socket, async_write(_, _))
//    //     .Times(AtLeast(1));
//    std::string path = "/static/a.txt";
//    handler->determineContentType(path);
//}
//// TEST_F(StaticFileHandlerTest, HandleRequest_sendfileChunk) {
////     std::string request = "GET /static/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
//
////     // EXPECT_CALL(*mock_socket, async_write(_, _))
////     //     .Times(AtLeast(1));
////     std::string path = "/static/a.txt";
////     handler->sendFileChunk();
////     io_service.run();
//// }