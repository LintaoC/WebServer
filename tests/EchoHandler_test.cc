#include "gtest/gtest.h"
#include "EchoHandler.h"
#include <boost/beast/http.hpp>

// Fixture for EchoHandler tests
class EchoHandlerTest : public ::testing::Test {
protected:
    EchoHandler echoHandler;
};

// Test that EchoHandler correctly echoes back the entire request
TEST_F(EchoHandlerTest, EchoesFullRequest) {
using namespace boost::beast::http;

// Create a request with a specific method, target, and body
request<string_body> req{verb::post, "/test", 11};
req.set(field::host, "localhost");
req.body() = "Test body content";
req.prepare_payload();  // Ensure the Content-Length header is correctly set

// Use the EchoHandler to handle the request
auto res = echoHandler.handle_request(req);

// Create the expected response body as a string that mirrors the full request
std::stringstream expected_response_body;
expected_response_body << req.method_string() << " " << req.target() << " HTTP/" << (req.version() / 10) << "." << (req.version() % 10) << "\r\n";
for (const auto& field : req.base()) {
expected_response_body << field.name_string() << ": " << field.value() << "\r\n";
}
expected_response_body << "\r\n" << req.body();

// Check that the response status is 200 OK
EXPECT_EQ(res.result(), status::ok);

// Check that the response body is exactly what was sent in the request, formatted as an HTTP message
EXPECT_EQ(res.body(), expected_response_body.str());

// Check that the Content-Type header is set to text/plain
EXPECT_EQ(res.base().at(field::content_type), "text/plain");
}