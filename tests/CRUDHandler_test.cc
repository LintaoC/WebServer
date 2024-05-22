#include "EntityDatabase.h"
#include "CRUDHandler.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/beast/http.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>

namespace http = boost::beast::http;

class MockEntityDatabase : public EntityDatabase {
public:
    MockEntityDatabase(const std::string& arg) : EntityDatabase(arg) {}

    MOCK_METHOD((std::pair<EntityDatabase::StatusCode, std::string>), create_entity, (const std::string &entity_name), (override));
    MOCK_METHOD(EntityDatabase::StatusCode, delete_entity, (const std::string &entity_name, const std::string &entity_id), (override));
    MOCK_METHOD(EntityDatabase::StatusCode, write_to_entity, (const std::string &entity_name, const std::string &entity_id, const std::string &content), (override));
    MOCK_METHOD((std::pair<EntityDatabase::StatusCode, std::string>), read_from_entity, (const std::string &entity_name, const std::string &entity_id), (const, override));
    MOCK_METHOD((std::pair<EntityDatabase::StatusCode, std::vector<std::string>>), list_entities, (const std::string &entity_name), (const, override));
};

class CRUDHandlerTest : public ::testing::Test {
protected:
    std::shared_ptr<MockEntityDatabase> db;
    std::shared_ptr<CRUDHandler> handler;

    void SetUp() override {
        db = std::make_shared<MockEntityDatabase>("../test_data");
        handler = std::make_shared<CRUDHandler>(db);
    }

    void TearDown() override {
        handler.reset();
        db.reset();
    }

    std::string normalize_json(const std::string& json_str) {
        std::stringstream ss;
        ss << json_str;
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(ss, pt);
        std::ostringstream oss;
        boost::property_tree::write_json(oss, pt, false); // Write without pretty print
        return oss.str();
    }
};

// Test POST request handling
TEST_F(CRUDHandlerTest, HandlePostValidRequest) {
    Request req;
    req.method(http::verb::post);
    req.target("/api/entity");
    req.body() = R"({"name": "test"})";
    req.prepare_payload();

    EXPECT_CALL(*db, create_entity("entity")).WillOnce(testing::Return(std::make_pair(EntityDatabase::Success, "1")));
    EXPECT_CALL(*db, write_to_entity("entity", "1", req.body())).WillOnce(testing::Return(EntityDatabase::Success));

    Response res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::created);
    EXPECT_EQ(res["Content-Type"], "application/json");

    std::string expected_body = R"({"id":"1"})";
    EXPECT_EQ(normalize_json(res.body()), normalize_json(expected_body));
}

// Test GET request handling for reading an entity by ID
TEST_F(CRUDHandlerTest, HandleGetEntityById) {
    Request req;
    req.method(http::verb::get);
    req.target("/api/entity/1");
    req.prepare_payload();

    EXPECT_CALL(*db, read_from_entity("entity", "1")).WillOnce(testing::Return(std::make_pair(EntityDatabase::Success, R"({"name": "test"})")));

    Response res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res["Content-Type"], "application/json");
    EXPECT_EQ(res.body(), R"({"name": "test"})");
}

// Test PUT request handling for updating an entity
TEST_F(CRUDHandlerTest, HandlePutValidRequest) {
    Request req;
    req.method(http::verb::put);
    req.target("/api/entity/1");
    req.body() = R"({"name": "updated"})";
    req.prepare_payload();

    EXPECT_CALL(*db, write_to_entity("entity", "1", req.body())).WillOnce(testing::Return(EntityDatabase::Success));

    Response res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.body(), "Entity updated successfully");
}

// Test DELETE request handling for deleting an entity
TEST_F(CRUDHandlerTest, HandleDeleteValidRequest) {
    Request req;
    req.method(http::verb::delete_);
    req.target("/api/entity/1");
    req.prepare_payload();

    EXPECT_CALL(*db, delete_entity("entity", "1")).WillOnce(testing::Return(EntityDatabase::Success));

    Response res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.body(), "Entity deleted successfully");
}

// Test POST request handling with invalid JSON
TEST_F(CRUDHandlerTest, HandlePostInvalidJson) {
    Request req;
    req.method(http::verb::post);
    req.target("/api/entity");
    req.body() = R"({"name": "test)"; // Invalid JSON
    req.prepare_payload();

    Response res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::bad_request);
    EXPECT_EQ(res.body(), "Invalid JSON format");
}

// Test GET request handling with invalid path
TEST_F(CRUDHandlerTest, HandleGetInvalidPath) {
    Request req;
    req.method(http::verb::get);
    req.target("/api/entity/1/extra"); // Invalid path
    req.prepare_payload();

    Response res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::bad_request);
    EXPECT_EQ(res.body(), "Invalid request path");
}

// Test GET request handling for non-existent entity
TEST_F(CRUDHandlerTest, HandleGetEntityNotFound) {
    Request req;
    req.method(http::verb::get);
    req.target("/api/entity/999"); // Non-existent entity
    req.prepare_payload();

    EXPECT_CALL(*db, read_from_entity("entity", "999")).WillOnce(testing::Return(std::make_pair(EntityDatabase::EntityIDNotFound, "")));

    Response res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.body(), "Entity not found");
}

// Test PUT request handling for non-existent entity
TEST_F(CRUDHandlerTest, HandlePutEntityNotFound) {
    Request req;
    req.method(http::verb::put);
    req.target("/api/entity/999"); // Non-existent entity
    req.body() = R"({"name": "updated"})";
    req.prepare_payload();

    EXPECT_CALL(*db, write_to_entity("entity", "999", req.body())).WillOnce(testing::Return(EntityDatabase::EntityIDNotFound));

    Response res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.body(), "Entity not found");
}

// Test DELETE request handling for non-existent entity
TEST_F(CRUDHandlerTest, HandleDeleteEntityNotFound) {
    Request req;
    req.method(http::verb::delete_);
    req.target("/api/entity/999"); // Non-existent entity
    req.prepare_payload();

    EXPECT_CALL(*db, delete_entity("entity", "999")).WillOnce(testing::Return(EntityDatabase::EntityIDNotFound));

    Response res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.body(), "Entity not found");
}

// Test handling of unsupported HTTP method
TEST_F(CRUDHandlerTest, HandleUnsupportedMethod) {
    Request req;
    req.method(http::verb::patch); // Unsupported method
    req.target("/api/entity/1");
    req.prepare_payload();

    Response res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::method_not_allowed);
    EXPECT_EQ(res.body(), "Method not allowed");
}

// Test GET request handling for listing entities
TEST_F(CRUDHandlerTest, HandleGetListEntities) {
    Request req;
    req.method(http::verb::get);
    req.target("/api/entity");
    req.prepare_payload();

    std::vector<std::string> entity_ids = {"1", "2", "3"};
    EXPECT_CALL(*db, list_entities("entity")).Times(1).WillOnce(testing::Return(std::make_pair(EntityDatabase::Success, entity_ids)));

    Response res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res["Content-Type"], "application/json");

    std::string expected_body = R"(["1","2","3"])";
    EXPECT_EQ(normalize_json(res.body()), normalize_json(expected_body));
}