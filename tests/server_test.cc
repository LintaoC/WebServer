 #include <boost/asio.hpp>
#include <gtest/gtest.h>
#include "server.h"
#include "session.h"
#include "ISessionFactory.h"
#include "RequestHandlerFactory.h"

// Mock session factory for testing
class MockSessionFactory : public ISessionFactory {
public:
    boost::shared_ptr<session> create(boost::asio::io_service& io_service, std::map<std::string, RequestHandlerFactory*>* routes) override {
        return boost::shared_ptr<session>(new session(io_service, routes));
    }
};

// Test fixture for the server tests
class ServerTest : public ::testing::Test {
protected:
    boost::asio::io_service io_service_;
    MockSessionFactory factory_;
    std::map<std::string, RequestHandlerFactory*> routes_;

    void SetUp() override {
        // Add some dummy route handlers if needed
        routes_["/echo"] = new RequestHandlerFactory();
        routes_["/static"] = new RequestHandlerFactory();
    }

    void TearDown() override {
        // Clean up route handlers
        for (auto& route : routes_) {
            delete route.second;
        }
    }
};

// Test the constructor of the server class
TEST_F(ServerTest, ConstructorTest) {
server s(io_service_, 8080, &factory_, &routes_);
EXPECT_EQ(s.acceptor_.local_endpoint().port(), 8080);
}

// Test the start_accept method
TEST_F(ServerTest, StartAcceptTest) {
server s(io_service_, 8080, &factory_, &routes_);
s.start_accept();
EXPECT_TRUE(s.acceptor_.is_open());
}

// Test the handle_accept method with an error
TEST_F(ServerTest, HandleAcceptErrorTest) {
server s(io_service_, 8080, &factory_, &routes_);
boost::shared_ptr<session> new_session = factory_.create(io_service_, &routes_);
boost::system::error_code ec = boost::asio::error::operation_aborted; // Simulate an error
s.handle_accept(new_session, ec);
EXPECT_FALSE(new_session->socket().is_open());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}