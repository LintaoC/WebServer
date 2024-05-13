#include "gtest/gtest.h"
#include "NotFoundHandler.h"
#include <boost/beast/http.hpp>

// Fixture for NotFoundHandler tests
class NotFoundHandlerTest : public ::testing::Test {
protected:
    NotFoundHandler notFoundHandler;
};

// Test that NotFoundHandler sets the correct status, headers, and body
TEST_F(NotFoundHandlerTest, ReturnsNotFoundResponse) {
using namespace boost::beast::http;

// Create a dummy request that would trigger a 404
request<string_body> req{verb::get, "/does-not-exist", 11};
req.prepare_payload();

// Use the NotFoundHandler to handle the request
auto res = notFoundHandler.handle_request(req);

// Check that the response status is 404 Not Found
EXPECT_EQ(res.result(), status::not_found);

// Check that the Content-Type header is set to text/html
EXPECT_EQ(res.base().at(field::content_type), "text/html");

// Check the body of the response
std::string expected_body = "<html><body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body></html>";
EXPECT_EQ(res.body(), expected_body);
}