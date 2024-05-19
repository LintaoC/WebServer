#include "gtest/gtest.h"
#include "RequestHandlerFactory.h"
#include "RequestHandler.h"
#include "EchoHandler.h"
#include "StaticFileHandler.h"
#include "NotFoundHandler.h"

class RequestHandlerFactoryTest : public ::testing::Test {
protected:
    std::string rootPath = "/var/www";
    std::string relativePath = "/images";
};

TEST_F(RequestHandlerFactoryTest, CreatesEchoHandler) {
    RequestHandlerFactory factory("EchoHandler", {});
    RequestHandler *handler = factory.buildRequestHandler();
    ASSERT_NE(nullptr, handler);
    EXPECT_TRUE(dynamic_cast<EchoHandler *>(handler) != nullptr);
    delete handler;
}

TEST_F(RequestHandlerFactoryTest, CreatesStaticFileHandler) {
    RequestHandlerFactory factory("StaticHandler", {{"root", rootPath}});
    factory.setRelativePath(relativePath);
    RequestHandler *handler = factory.buildRequestHandler();
    ASSERT_NE(nullptr, handler);
    StaticFileHandler *staticHandler = dynamic_cast<StaticFileHandler *>(handler);
    EXPECT_TRUE(staticHandler != nullptr);
    delete handler;
}

TEST_F(RequestHandlerFactoryTest, CreatesNotFoundHandler) {
    RequestHandlerFactory factory("NotFoundHandler", {});
    RequestHandler *handler = factory.buildRequestHandler();
    ASSERT_NE(nullptr, handler);
    EXPECT_TRUE(dynamic_cast<NotFoundHandler *>(handler) != nullptr);
    delete handler;
}

TEST_F(RequestHandlerFactoryTest, HandlesUnknownHandlerType) {
    RequestHandlerFactory factory("InvalidHandler", {});
    RequestHandler *handler = factory.buildRequestHandler();
    ASSERT_NE(nullptr, handler);
    EXPECT_TRUE(dynamic_cast<NotFoundHandler *>(handler) != nullptr);
    delete handler;
}

TEST_F(RequestHandlerFactoryTest, CorrectlyGetsHandlerType) {
    RequestHandlerFactory factory("EchoHandler", {});
    EXPECT_EQ("EchoHandler", factory.getHandlerType());
}

TEST_F(RequestHandlerFactoryTest, CorrectlyGetsAndSetsRelativePath) {
    RequestHandlerFactory factory("StaticHandler", {{"root", rootPath}});
    factory.setRelativePath(relativePath);
    EXPECT_EQ(relativePath, factory.getRelativePath());
}

TEST_F(RequestHandlerFactoryTest, CorrectlyGetsHandlerParams) {
    std::map<std::string, std::string> params = {
            {"root", rootPath},
            {"arg1", "val1"},
            {"arg2", "val2"}
    };
    auto expected_params = params;
    expected_params["relative_path"] = relativePath;

    RequestHandlerFactory factory("StaticHandler", params);
    factory.setRelativePath(relativePath);
    std::map<std::string, std::string> handlerParams = factory.getHandlerParams();

    EXPECT_EQ(expected_params, handlerParams);
}
