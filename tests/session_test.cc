#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "../include/session.h"
#include "../include/RequestHandlerFactory.h"
#include "../include/RequestHandler.h"
#include <boost/beast/http.hpp>
#include <memory>

using boost::asio::ip::tcp;

// Mock class for RequestHandler
class MockRequestHandler : public RequestHandler {
public:
    MockRequestHandler() = default;

    boost::beast::http::response<boost::beast::http::string_body> handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) override {
        boost::beast::http::response<boost::beast::http::string_body> res;
        res.result(boost::beast::http::status::ok);
        res.body() = "Mock response";
        res.prepare_payload();
        return res;
    }
};

// Mock class for RequestHandlerFactory
class MockRequestHandlerFactory : public RequestHandlerFactory {
public:
    MockRequestHandlerFactory() : RequestHandlerFactory("", {}) {}

    RequestHandler* buildRequestHandler() const override {
        return new MockRequestHandler();
    }
};

// Test fixture for session
class SessionTest : public ::testing::Test {
protected:
    boost::asio::io_service io_service;
    std::map<std::string, RequestHandlerFactory*> routes;

    void SetUp() override {
        routes["/mock"] = new MockRequestHandlerFactory();
    }

    void TearDown() override {
        for (auto& route : routes) {
            delete route.second;
        }
    }

    std::shared_ptr<session> create_session() {
        return std::make_shared<session>(io_service, &routes);
    }

    void simulate_read(std::shared_ptr<session> s, const std::string& request) {
        boost::system::error_code ec;
        std::memcpy(s->data_, request.c_str(), request.size());

        io_service.post([s, ec, request_size = request.size()]() {
            s->handle_read(ec, request_size);
        });

        io_service.run();
        io_service.reset();
    }
};

TEST_F(SessionTest, ValidateRequest_Valid) {
    auto sess = create_session();
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::get);
    req.set(boost::beast::http::field::host, "localhost");

    EXPECT_TRUE(sess->validate_request(req));
}

TEST_F(SessionTest, ValidateRequest_MissingHost) {
    auto sess = create_session();
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::get);

    EXPECT_FALSE(sess->validate_request(req));
}

TEST_F(SessionTest, ValidateRequest_DuplicateHost) {
    auto sess = create_session();
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::get);
    req.set(boost::beast::http::field::host, "localhost");
    req.insert(boost::beast::http::field::host, "localhost");

    EXPECT_FALSE(sess->validate_request(req));
}

TEST_F(SessionTest, ValidateRequest_InvalidCharactersInHeader) {
    auto sess = create_session();
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::get);
    req.set(boost::beast::http::field::host, "localhost");
    req.set("Invalid-Header-Name-!", "value");

    EXPECT_FALSE(sess->validate_request(req));
}

TEST_F(SessionTest, ValidateRequest_NonAsciiURI) {
    auto sess = create_session();
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::get);
    req.set(boost::beast::http::field::host, "localhost");
    req.target("/nonascii\xe2\x28\xa1");

    EXPECT_FALSE(sess->validate_request(req));
}

TEST_F(SessionTest, ValidateRequest_MissingContentLengthInPost) {
    auto sess = create_session();
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::post);
    req.set(boost::beast::http::field::host, "localhost");

    EXPECT_FALSE(sess->validate_request(req));
}

TEST_F(SessionTest, GetRequestHandlerFactory_NoMatch) {
    auto sess = create_session();
    std::string path = "/nomatch";
    RequestHandlerFactory* factory = sess->getRequestHandlerFactory(path, &routes);

    EXPECT_NE(factory, nullptr);
    delete factory;  // Clean up the allocated factory
}

TEST_F(SessionTest, GetRequestHandlerFactory_Match) {
    auto sess = create_session();
    std::string path = "/mock/path";
    RequestHandlerFactory* factory = sess->getRequestHandlerFactory(path, &routes);

    EXPECT_EQ(factory, routes["/mock"]);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
