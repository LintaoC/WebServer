#include <gtest/gtest.h>
#include <boost/beast/http.hpp>
#include "HealthHandler.h"

// Test fixture for HealthHandler
class HealthHandlerTest : public ::testing::Test {
protected:
    HealthHandler handler;

    // Helper function to create a sample HTTP request
    boost::beast::http::request<boost::beast::http::string_body> create_request() {
        boost::beast::http::request<boost::beast::http::string_body> req;
        req.method(boost::beast::http::verb::get);
        req.target("/health");
        req.version(11); // HTTP/1.1
        return req;
    }
};

// Test case for HealthHandler's handle_request method
TEST_F(HealthHandlerTest, HandleRequest) {
    auto req = create_request();
    auto res = handler.handle_request(req);

    // Check the status code
    EXPECT_EQ(res.result(), boost::beast::http::status::ok);

    // Check the content type
    EXPECT_EQ(res[boost::beast::http::field::content_type], "text/plain");

    // Check the response body
    EXPECT_EQ(res.body(), "OK");

    // Ensure the response is properly prepared (content length should be set)
    EXPECT_TRUE(res.has_content_length());
}