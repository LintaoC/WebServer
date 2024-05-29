#include "gtest/gtest.h"
#include "SleepRequestHandler.h"
#include <boost/beast/http.hpp>
#include <chrono>

// Fixture for SleepRequestHandler tests
class SleepRequestHandlerTest : public ::testing::Test {
protected:
    SleepRequestHandler sleepHandler;
};

// Test that SleepRequestHandler correctly sleeps and responds
TEST_F(SleepRequestHandlerTest, SleepsAndResponds) {
    using namespace boost::beast::http;

    // Create a request with a specific method, target, and version
    request<string_body> req{verb::get, "/sleep", 11};
    req.set(field::host, "localhost");

    // Start measuring the time
    auto start_time = std::chrono::steady_clock::now();

    // Use the SleepRequestHandler to handle the request
    auto res = sleepHandler.handle_request(req);

    // Stop measuring the time
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

    // Check that the handler slept for at least 5 seconds
    EXPECT_GE(duration, 5);

    // Check that the response status is 200 OK
    EXPECT_EQ(res.result(), status::ok);

    // Check that the response body is the expected message
    EXPECT_EQ(res.body(), "Sleep Request Handler Response");

    // Check that the Content-Type header is set to text/plain
    EXPECT_EQ(res.base().at(field::content_type), "text/plain");

    // Check that the Server header is set to "TestServer"
    EXPECT_EQ(res.base().at(field::server), "TestServer");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
