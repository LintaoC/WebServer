// #include <gtest/gtest.h>
// #include <gmock/gmock.h>
// #include <boost/asio.hpp>
// #include <boost/beast/http.hpp>
// #include <boost/log/trivial.hpp>
// #include <boost/log/utility/setup/common_attributes.hpp>
// #include <boost/log/utility/setup/console.hpp>
// #include "../include/session.h"
// #include "../include/RequestHandlerFactory.h"
// #include "../include/RequestHandler.h"

// using namespace testing;
// using namespace boost::asio;
// using namespace boost::beast::http;
// using ip::tcp;

// // Mock class for RequestHandler
// class MockRequestHandler : public RequestHandler {
// public:
//     MOCK_METHOD(response<string_body>, handle_request, (const request<string_body>& req), (override));
// };

// // Mock class for RequestHandlerFactory
// class MockRequestHandlerFactory : public RequestHandlerFactory {
// public:
//     MockRequestHandlerFactory(const std::string& type, const std::string& path)
//         : RequestHandlerFactory(type, path) {}

//     MOCK_METHOD(RequestHandler*, buildRequestHandler, (), (const, override));
// };

// // Mock class for Session
// class MockSession : public session {
// public:
//     MockSession(boost::asio::io_service& io_service, std::map<std::string, RequestHandlerFactory*>* routes)
//         : session(io_service, routes), socket_(io_service) {}

//     MOCK_METHOD0(start, void());
//     MOCK_METHOD(void, handle_read, (const boost::system::error_code& error, size_t bytes_transferred), (override));

//     tcp::socket& socket() { return socket_; }

// private:
//     tcp::socket socket_;
// };

// class SessionTest : public Test {
// protected:
//     io_service io_service_;
//     std::map<std::string, RequestHandlerFactory*> routes_;
//     std::shared_ptr<session> session_;
//     MockRequestHandlerFactory* mock_factory_;

//     void SetUp() override {
//         mock_factory_ = new MockRequestHandlerFactory("EchoHandler", "/echo");
//         routes_["/echo"] = mock_factory_;
//         session_ = std::make_shared<session>(io_service_, &routes_);

//         // Initialize logging
//         boost::log::add_common_attributes();
//         boost::log::add_console_log(std::cout, boost::log::keywords::format = "%TimeStamp% [%Severity%]: %Message%");
//     }

//     void TearDown() override {
//         delete mock_factory_;
//     }
// };

// TEST_F(SessionTest, Start) {
//     auto endpoint = tcp::endpoint(tcp::v4(), 8080);
//     session_->socket().open(endpoint.protocol());
//     session_->socket().bind(endpoint);

//     auto acceptor = std::make_shared<tcp::acceptor>(io_service_, endpoint);
//     acceptor->listen();

//     acceptor->async_accept(session_->socket(), [&](const boost::system::error_code& ec) {
//         if (!ec) {
//             session_->start();
//             io_service_.stop();
//         }
//     });

//     io_service_.run();
//     // EXPECT_TRUE(session_->socket().is_open());
// }

// // TEST_F(SessionTest, HandleReadSuccess) {
// //     auto mock_handler = new MockRequestHandler();
// //     EXPECT_CALL(*mock_factory_, buildRequestHandler()).WillOnce(Return(mock_handler));

// //     std::string request_str = "GET /echo HTTP/1.1\r\nHost: localhost\r\n\r\n";
// //     std::copy(request_str.begin(), request_str.end(), session_->data_);

// //     boost::system::error_code ec;
// //     size_t bytes_transferred = request_str.size();
// //     EXPECT_CALL(*mock_handler, handle_request(_)).WillOnce(Return(
// //         response<string_body>{status::ok, 11, "Hello, world!"}
// //     ));

// //     session_->handle_read(ec, bytes_transferred);
// //     EXPECT_TRUE(session_->socket().is_open());

// //     delete mock_handler;
// // }

// // TEST_F(SessionTest, HandleReadError) {
// //     boost::system::error_code ec = boost::asio::error::connection_reset;
// //     size_t bytes_transferred = 0;

// //     session_->handle_read(ec, bytes_transferred);
// //     EXPECT_FALSE(session_->socket().is_open());
// // }

// // TEST_F(SessionTest, GetRequestHandlerFactory) {
// //     auto factory = session_->getRequestHandlerFactory("/echo", &routes_);
// //     EXPECT_EQ(factory, mock_factory_);
// // }

// // TEST_F(SessionTest, HandleAcceptError) {
// //     BOOST_LOG_TRIVIAL(info) << "Test HandleAcceptError begins";
// //     auto mock_session = std::make_shared<MockSession>(io_service_, &routes_);
    
// //     // Simulate an error during acceptance
// //     boost::system::error_code ec = make_error_code(boost::asio::error::connection_refused);

// //     EXPECT_CALL(*mock_session, start()).Times(0);

// //     // Directly call handle_read with an error
// //     session_->handle_read(ec, 0);

// //     BOOST_LOG_TRIVIAL(info) << "Test HandleAcceptError ends";
// // }

// int main(int argc, char **argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     ::testing::InitGoogleMock(&argc, argv);

//     // Initialize Boost.Log
//     boost::log::add_common_attributes();
//     boost::log::add_console_log(std::cout, boost::log::keywords::format = "%TimeStamp% [%Severity%]: %Message%");
    
//     return RUN_ALL_TESTS();
// }
