#include <gtest/gtest.h>
#include <boost/beast/http.hpp>
#include "GeneratorHandler.h"

class GeneratorHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any necessary resources for the test
    }

    void TearDown() override {
        // Clean up any resources used by the test
    }
};

TEST_F(GeneratorHandlerTest, HandleRequestGeneratesImage) {
    // Prepare the request
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::post);
    req.target("/generator");
    req.set(boost::beast::http::field::content_type, "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");
    req.body() = "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
                 "Content-Disposition: form-data; name=\"description\"\r\n\r\n"
                 "apple\r\n"
                 "------WebKitFormBoundary7MA4YWxkTrZu0gW--";
    req.prepare_payload();

    // Set up the handler
    std::map<std::string, std::string> params;
    GeneratorHandler handler(params);

    // Handle the request
    auto res = handler.handle_request(req);

    // Verify the response
    EXPECT_EQ(res.result(), boost::beast::http::status::ok);
    EXPECT_EQ(res[boost::beast::http::field::content_type], "image/png");
    EXPECT_FALSE(res.body().empty());
}

TEST_F(GeneratorHandlerTest, HandleRequestMissingDescription) {
    // Prepare the request with missing description
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::post);
    req.target("/generator");
    req.set(boost::beast::http::field::content_type, "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");
    req.body() = "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
                 "Content-Disposition: form-data; name=\"description\"\r\n\r\n"
                 "\r\n"
                 "------WebKitFormBoundary7MA4YWxkTrZu0gW--";
    req.prepare_payload();

    // Set up the handler
    std::map<std::string, std::string> params;
    GeneratorHandler handler(params);

    // Handle the request
    auto res = handler.handle_request(req);

    // Verify the response
    EXPECT_EQ(res.result(), boost::beast::http::status::internal_server_error);
    EXPECT_EQ(res[boost::beast::http::field::content_type], "text/html");
    EXPECT_FALSE(res.body().empty());
}



int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
