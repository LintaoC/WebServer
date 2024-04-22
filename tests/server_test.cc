#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "server.h"
#include "session.h"
#include "ISessionFactory.h"

using namespace testing;
using boost::asio::ip::tcp;

// Mocksession overrides the start method of the session class
class MockSession : public session
{
public:
    explicit MockSession(boost::asio::io_service &io_service) : session(io_service) {}
    MOCK_METHOD(void, start, (), (override)); // Mock the start method
};

// create is used to create a session object
class MockSessionFactory : public ISessionFactory
{
public:
    MOCK_METHOD(session *, create, (boost::asio::io_service &), (override)); // Mock the create method
};

class ServerTest : public Test
{
protected:
    boost::asio::io_service io_service;
    std::unique_ptr<boost::asio::io_service::work> work;
    std::unique_ptr<server> test_server;
    std::shared_ptr<MockSession> mock_session;
    MockSessionFactory mock_factory;
    boost::asio::steady_timer timer;
    short test_port = 8080;

    // Constructor - initializes the timer with the io_service
    ServerTest() : timer(io_service) {}

    // call setup before each test
    void SetUp() override
    {
        work = std::make_unique<boost::asio::io_service::work>(io_service); // Keep io_service running
        mock_session = std::make_shared<MockSession>(io_service);           // Create a mock session
        // Setup the mock factory to return the mock session
        EXPECT_CALL(mock_factory, create(_)).WillRepeatedly(Return(mock_session.get())); // Return the mock session when create is called
        ASSERT_NE(mock_session.get(), nullptr);                                          // Ensure the mock session was created
        EXPECT_CALL(*mock_session, start()).Times(AtLeast(1));                           // Expect start to be called at least once

        // Create the server with the mock factory
        test_server = std::make_unique<server>(io_service, test_port, &mock_factory);

        // Setup a timer to prevent test from hanging indefinitely
        timer.expires_after(std::chrono::seconds(1));
        timer.async_wait([this](const boost::system::error_code &)
                         {
                             io_service.stop(); // Forcefully stop io_service after 1 second if not stopped
                         });
    }

    // call teardown after each test
    void TearDown() override
    {
        io_service.run(); // Ensure all work is finished
        io_service.reset();
        work.reset(); // Allow io_service to exit
    }
};

TEST_F(ServerTest, AcceptsConnectionAndStartsSession)
{
    // io_service.run(); // This will block until all work has finished or io_service is stopped
    boost::system::error_code ec;                       // simulate successful connection
    test_server->handle_accept(mock_session.get(), ec); // Directly invoke to test the flow
    io_service.run();
}