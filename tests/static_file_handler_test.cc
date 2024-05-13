#include "gtest/gtest.h"
#include "StaticFileHandler.h"
#include <boost/beast/http.hpp>

class StaticFileHandlerTest : public ::testing::Test {
protected:
    std::string rootPath = "./tests/test_configs";
    StaticFileHandler* handler;

    virtual void TearDown() {
        delete handler;
    }
};

TEST_F(StaticFileHandlerTest, HandlesFileFound) {
std::string relativePath = "/test.txt";
handler = new StaticFileHandler(rootPath, relativePath);

using namespace boost::beast::http;

// Simulate a request for the test file
request<string_body> req{verb::get, relativePath, 11};
auto res = handler->handle_request(req);

// Check response status code
EXPECT_EQ(res.result(), status::ok);

// Check content type
EXPECT_EQ(res.base().at(field::content_type), "text/plain");

// Check response body
EXPECT_EQ(res.body(), "Hello, world!");
}

TEST_F(StaticFileHandlerTest, HandlesFileNotFound) {
std::string nonexistentPath = "/nonexistent.txt";
handler = new StaticFileHandler(rootPath, nonexistentPath);

using namespace boost::beast::http;

// Simulate a request for a nonexistent file
request<string_body> req{verb::get, nonexistentPath, 11};
auto res = handler->handle_request(req);

// Check response status code
EXPECT_EQ(res.result(), status::not_found);

// Check content type
EXPECT_EQ(res.base().at(field::content_type), "text/html");

// Check response body
std::string expectedBody = "<html><body><h1>404 FileNotFound</h1></body></html>";
EXPECT_EQ(res.body(), expectedBody);
}
