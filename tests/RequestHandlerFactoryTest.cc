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
RequestHandlerFactory factory("EchoHandler", rootPath);
RequestHandler* handler = factory.buildRequestHandler();
ASSERT_NE(nullptr, handler);
EXPECT_TRUE(dynamic_cast<EchoHandler*>(handler) != nullptr);
delete handler;
}

TEST_F(RequestHandlerFactoryTest, CreatesStaticFileHandler) {
RequestHandlerFactory factory("StaticHandler", rootPath);
factory.setRelativePath(relativePath);
RequestHandler* handler = factory.buildRequestHandler();
ASSERT_NE(nullptr, handler);
StaticFileHandler* staticHandler = dynamic_cast<StaticFileHandler*>(handler);
EXPECT_TRUE(staticHandler != nullptr);
delete handler;
}

TEST_F(RequestHandlerFactoryTest, CreatesNotFoundHandler) {
RequestHandlerFactory factory("NotFoundHandler", rootPath);
RequestHandler* handler = factory.buildRequestHandler();
ASSERT_NE(nullptr, handler);
EXPECT_TRUE(dynamic_cast<NotFoundHandler*>(handler) != nullptr);
delete handler;
}

TEST_F(RequestHandlerFactoryTest, HandlesUnknownHandlerType) {
RequestHandlerFactory factory("InvalidHandler", rootPath);
RequestHandler* handler = factory.buildRequestHandler();
ASSERT_NE(nullptr, handler);
EXPECT_TRUE(dynamic_cast<NotFoundHandler*>(handler) != nullptr);
delete handler;
}

TEST_F(RequestHandlerFactoryTest, CorrectlyGetsHandlerType) {
RequestHandlerFactory factory("EchoHandler", rootPath);
EXPECT_EQ("EchoHandler", factory.getHandlerType());
}

TEST_F(RequestHandlerFactoryTest, CorrectlyGetsRootPath) {
RequestHandlerFactory factory("EchoHandler", rootPath);
EXPECT_EQ(rootPath, factory.getRootPath());
}

TEST_F(RequestHandlerFactoryTest, CorrectlyGetsAndSetsRelativePath) {
RequestHandlerFactory factory("StaticHandler", rootPath);
factory.setRelativePath(relativePath);
EXPECT_EQ(relativePath, factory.getRelativePath());
}