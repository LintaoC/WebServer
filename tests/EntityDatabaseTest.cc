#include "gtest/gtest.h"
#include "EntityDatabase.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class EntityDatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up a temporary directory for testing
        test_data_path = "test_data";
        fs::remove_all(test_data_path);  // Clean up any previous test data
        fs::create_directories(test_data_path);
        db = std::make_unique<EntityDatabase>(test_data_path);
    }

    void TearDown() override {
        // Clean up test data after each test
        fs::remove_all(test_data_path);
    }

    std::unique_ptr<EntityDatabase> db;
    std::string test_data_path;
};

TEST_F(EntityDatabaseTest, CreateEntityTest) {
    auto result = db->create_entity("TestEntity");
    EXPECT_EQ(result.first, EntityDatabase::Success);
    EXPECT_FALSE(result.second.empty());  // UUID should not be empty

    fs::path filepath(test_data_path + "/TestEntity/" + result.second);
    EXPECT_TRUE(fs::exists(filepath));  // File should be created
}

TEST_F(EntityDatabaseTest, DeleteEntityTest) {
    auto create_result = db->create_entity("TestEntity");
    auto id = create_result.second;

    auto delete_result = db->delete_entity("TestEntity", id);
    EXPECT_EQ(delete_result, EntityDatabase::Success);

    fs::path filepath(test_data_path + "/TestEntity/" + id);
    EXPECT_FALSE(fs::exists(filepath));  // File should be deleted
}

TEST_F(EntityDatabaseTest, WriteToEntityTest) {
    auto create_result = db->create_entity("TestEntity");
    auto id = create_result.second;

    std::string content = "This is a test content.";
    auto write_result = db->write_to_entity("TestEntity", id, content);
    EXPECT_EQ(write_result, EntityDatabase::Success);

    fs::path filepath(test_data_path + "/TestEntity/" + id);
    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    EXPECT_EQ(buffer.str(), content);  // Content should match
}

TEST_F(EntityDatabaseTest, ReadFromEntityTest) {
    auto create_result = db->create_entity("TestEntity");
    auto id = create_result.second;

    std::string content = "This is a test content.";
    db->write_to_entity("TestEntity", id, content);

    auto read_result = db->read_from_entity("TestEntity", id);
    EXPECT_EQ(read_result.first, EntityDatabase::Success);
    EXPECT_EQ(read_result.second, content);  // Content should match
}

TEST_F(EntityDatabaseTest, ListEntitiesTest) {
    db->create_entity("TestEntity");
    db->create_entity("TestEntity");

    auto list_result = db->list_entities("TestEntity");
    EXPECT_EQ(list_result.first, EntityDatabase::Success);
    EXPECT_EQ(list_result.second.size(), 2);  // Should list 2 entities
}

TEST_F(EntityDatabaseTest, EntityTypeNotFoundTest) {
    auto delete_result = db->delete_entity("NonExistentEntity", "some_id");
    EXPECT_EQ(delete_result, EntityDatabase::EntityTypeNotFound);

    auto read_result = db->read_from_entity("NonExistentEntity", "some_id");
    EXPECT_EQ(read_result.first, EntityDatabase::EntityTypeNotFound);

    auto list_result = db->list_entities("NonExistentEntity");
    EXPECT_EQ(list_result.first, EntityDatabase::EntityTypeNotFound);
}

TEST_F(EntityDatabaseTest, EntityIDNotFoundTest) {
    db->create_entity("TestEntity");

    auto delete_result = db->delete_entity("TestEntity", "non_existent_id");
    EXPECT_EQ(delete_result, EntityDatabase::EntityIDNotFound);

    auto read_result = db->read_from_entity("TestEntity", "non_existent_id");
    EXPECT_EQ(read_result.first, EntityDatabase::EntityIDNotFound);

    auto write_result = db->write_to_entity("TestEntity", "non_existent_id", "some_content");
    EXPECT_EQ(write_result, EntityDatabase::EntityIDNotFound);
}

TEST_F(EntityDatabaseTest, FileSystemErrorTest) {
    auto result = db->create_entity("TestEntity");
    auto id = result.second;

    // Simulate a filesystem error by removing the file after creation
    fs::remove(test_data_path + "/TestEntity/" + id);
    auto write_result = db->write_to_entity("TestEntity", id, "some_content");
    EXPECT_EQ(write_result, EntityDatabase::EntityIDNotFound);
}