<<<<<<< PATCH SET (531c6a modify the server class and increase the coverage rate)
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
=======
// #include "gtest/gtest.h"
// #include "gmock/gmock.h"
// #include <boost/asio.hpp>
// #include "../include/session.h"

// using namespace testing;
// using boost::asio::ip::tcp;

// class MockSession : public session
// {
// public:
//     MockSession(boost::asio::io_service &io_service)
//         : session(io_service) {}
//     MOCK_METHOD(void, handle_read, (const boost::system::error_code &error, size_t bytes_transferred), (override));
//     MOCK_METHOD(void, send_response, (), (override));
// };

// class SessionTest : public Test
// {
// protected:
//     boost::asio::io_service io_service;
//     std::unique_ptr<session> test_session;

//     void SetUp() override
//     {
//         test_session = std::make_unique<session>(io_service);
//     }
// };

// TEST_F(SessionTest, SessionStartsCorrectly)
// {
//     EXPECT_NO_THROW(test_session->start());
//     // You can add more checks to verify that async_read_some is called
// }

// TEST_F(SessionTest, HandleReadWithCompleteData) {
//     std::string valid_request = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n";
//     boost::system::error_code ec;  // no error
//     test_session->handle_read(ec, valid_request.size());
//     // Check if send_response is triggered
// }

// TEST_F(SessionTest, HandleReadWithIncompleteData) {
//     std::string incomplete_request = "GET / HTTP/1.1\r\nHost: ";
//     boost::system::error_code ec;  // no error
//     test_session->handle_read(ec, incomplete_request.size());
//     // Check if it waits for more data
// }
// TEST_F(SessionTest, HandleReadWithMultipleDataChunks) {
//     std::string first_chunk = "GET / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: ";
//     std::string second_chunk = "20\r\n\r\nThis is the body data";
//     boost::system::error_code ec;  // no error

//     // First chunk received
//     test_session->handle_read(ec, first_chunk.size());
//     // Second chunk completes the request
//     test_session->handle_read(ec, second_chunk.size());

//     // Check if the complete request triggers the response correctly.
//     // This might include verifying that `send_response` is eventually called.
// }
// TEST_F(SessionTest, SendCompleteResponse) {
//     std::string complete_request = "POST /submit-form HTTP/1.1\r\n"
//                                     "Host: www.example.com\r\n"
//                                     "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36\r\n"

//                                     "username=johndoe&password=123";
//     boost::system::error_code ec; 
//     test_session->handle_read(ec, complete_request.size());

//     // Since `send_response` is a complex function involving async operations,
//     // you might want to use mocks to verify that `async_write` is called with the correct data.
// }
// TEST_F(SessionTest, HandlePartialHeaders) {
//     std::string incomplete_header = "GET / HTTP/1.1\r\nHost:";
//     boost::system::error_code ec;  // no error
//     test_session->handle_read(ec, incomplete_header.size());

//     // Verify that the system correctly waits for more data rather than attempting to process an incomplete request.
// }
// // TEST_F(SessionTest, HandleCompleteRequestWithProperHeadersAndBody) {
// //     std::string header_part = "GET / HTTP/1.1\r\n"
// //                               "Host: www.example.com\r\n"
// //                               "Content-Type: text/html\r\n"
// //                               "Content-Length: 27\r\n"
// //                               "Connection: keep-alive\r\n\r\n";
// //     std::string body_part = "username=johndoe&password=123";

// //     boost::system::error_code ec;  // Simulate no error

// //     // Mimic receiving the header part first
// //     memcpy(test_session->data_, header_part.c_str(), header_part.size());
// //     test_session->handle_read(ec, header_part.size());

// //     // Now mimic receiving the body part
// //     memcpy(test_session->data_, body_part.c_str(), body_part.size());
// //     test_session->handle_read(ec, body_part.size());

// //     // Attempt to run the io_service to process any remaining asynchronous operations
// //   // Reset io_service after running to clear the state for potential subsequent operations.

// //     // At this point, your response should have been constructed and sent if all conditions were met
// // }
// TEST_F(SessionTest, HandleCompleteRequestWithProperHeadersAndBody) {
//     std::string complete_request = "GET / HTTP/1.1\r\n"
//                                    "Host: www.example.com\r\n"
//                                    "Content-Type: text/html\r\n"
//                                    "Content-Length: 27\r\n"
//                                    "Connection: keep-alive\r\n\r\n"
//                                    "username=johndoe&password=123";
//     boost::system::error_code ec;  // Simulate no error

//     // Mimic reading data into session
//     memcpy(test_session->data_, complete_request.data(), complete_request.size());
//     test_session->handle_read(ec, complete_request.size());

//     // Normally, you'd need to run io_service to process the async handlers
//     // Here we assume handle_read schedules all necessary operations synchronously
//     // which might not be true and usually isn't in real asynchronous systems
//     //io_service.run();  // To trigger any pending async operations like async_write

//     // Ensure io_service has no more work to prevent test from hanging
//     //io_service.reset();
// }
// TEST_F(SessionTest, HandleCompleteRequestTriggersResponse) {
//     // Complete HTTP request with headers and body
//     std::string complete_request = "POST /submit-form HTTP/1.1\r\n"
//                                    "Host: www.example.com\r\n"
//                                    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36\r\n"

//                                    "username=johndoe&password=123";

//     boost::system::error_code ec;  // no error
//     // Simulate reading the complete request in one go
//     test_session->handle_read(ec, complete_request.size());

//     // Verify that the complete request triggers the response correctly.
//     // This might include verifying that `send_response` is eventually called.
//     // Note: In a real-world scenario, you might want to mock `async_write` to check if it's called correctly,
//     //       but since `send_response` involves async operations, you might need to run the `io_service` to process it.
//     io_service.run();  // Process any pending asynchronous operations.

//     // Assertions can be more detailed, depending on how `send_response` is implemented.
//     // For example, checking internal state changes or outputs to a mock network interface.
// }
// TEST_F(SessionTest, HandleReadWithError) {
//     boost::system::error_code ec = boost::asio::error::make_error_code(boost::asio::error::network_down);
//     test_session->handle_read(ec, 0);
//     // Check if session handles errors correctly, perhaps closing the connection
// }

// TEST_F(SessionTest, ProcessCompleteRequestAndSendResponse) {
//     std::string full_request = "GET / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 0\r\n\r\n";
//     size_t request_length = full_request.size();

//     // Simulate reading the full request
//     test_session->handle_read(boost::system::error_code(), request_length);

//     // Ensure response is sent after a complete request
//     // Assuming `send_response` modifies some internal state or calls `async_write`
//     EXPECT_CALL(*test_session, send_response()).Times(1);

//     // Run io_service to complete all async operations
//     io_service.run();
// }

// TEST_F(SessionTest, HandleIncompleteRequest) {
//     std::string partial_request = "GET / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 15\r\n\r\npartial_body";
//     size_t partial_length = partial_request.size();

//     // Assuming `handle_read` is designed to queue more reads if the request isn't complete
//     test_session->handle_read(boost::system::error_code(), partial_length);

//     // The method should not attempt to send a response yet
//     EXPECT_CALL(*test_session, send_response()).Times(0);

//     // Run io_service to check for any further async operations that might have been queued
//     io_service.run();
// }
>>>>>>> BASE      (5c114a Revert "modify the session_test code in order to test the ne)
