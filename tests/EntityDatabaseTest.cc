#include "gtest/gtest.h"
#include "EntityDatabase.h"
#include <boost/filesystem.hpp>
#include <fstream>

// Fixture for EntityDatabase tests
class EntityDatabaseTest : public ::testing::Test {
protected:
    // Temporary directory for testing
    std::string temp_dir = "test_data";

    // EntityDatabase instance
    EntityDatabase db;

    EntityDatabaseTest() : db(temp_dir) {}

    void SetUp() override {
        // Create the temporary directory for testing
        boost::filesystem::create_directory(temp_dir);
    }

    void TearDown() override {
        // Remove the temporary directory and its contents after each test
        boost::filesystem::remove_all(temp_dir);
    }
};

// Test creating a new entity
TEST_F(EntityDatabaseTest, CreateEntity) {
    auto result = db.create_entity("TestEntity");
    EXPECT_EQ(result.first, EntityDatabase::Success);
    EXPECT_FALSE(result.second.empty());  // Check that the ID is not empty

    // Check that the file was created
    boost::filesystem::path filepath(temp_dir + "/TestEntity/" + result.second);
    EXPECT_TRUE(boost::filesystem::exists(filepath));
}

// Test deleting an entity
TEST_F(EntityDatabaseTest, DeleteEntity) {
    auto create_result = db.create_entity("TestEntity");
    EXPECT_EQ(create_result.first, EntityDatabase::Success);

    auto delete_result = db.delete_entity("TestEntity", create_result.second);
    EXPECT_EQ(delete_result, EntityDatabase::Success);

    // Check that the file was deleted
    boost::filesystem::path filepath(temp_dir + "/TestEntity/" + create_result.second);
    EXPECT_FALSE(boost::filesystem::exists(filepath));
}

// Test writing to an entity
TEST_F(EntityDatabaseTest, WriteToEntity) {
    auto create_result = db.create_entity("TestEntity");
    EXPECT_EQ(create_result.first, EntityDatabase::Success);

    std::string content = "Hello, world!";
    auto write_result = db.write_to_entity("TestEntity", create_result.second, content);
    EXPECT_EQ(write_result, EntityDatabase::Success);

    // Check that the content was written correctly
    std::ifstream file(temp_dir + "/TestEntity/" + create_result.second);
    std::stringstream buffer;
    buffer << file.rdbuf();
    EXPECT_EQ(buffer.str(), content);
}

// Test reading from an entity
TEST_F(EntityDatabaseTest, ReadFromEntity) {
    auto create_result = db.create_entity("TestEntity");
    EXPECT_EQ(create_result.first, EntityDatabase::Success);

    std::string content = "Hello, world!";
    db.write_to_entity("TestEntity", create_result.second, content);

    auto read_result = db.read_from_entity("TestEntity", create_result.second);
    EXPECT_EQ(read_result.first, EntityDatabase::Success);
    EXPECT_EQ(read_result.second, content);
}

// Test listing entities
TEST_F(EntityDatabaseTest, ListEntities) {
    auto create_result1 = db.create_entity("TestEntity");
    EXPECT_EQ(create_result1.first, EntityDatabase::Success);

    auto create_result2 = db.create_entity("TestEntity");
    EXPECT_EQ(create_result2.first, EntityDatabase::Success);

    auto list_result = db.list_entities("TestEntity");
    EXPECT_EQ(list_result.first, EntityDatabase::Success);

    std::vector<std::string> expected_ids = {create_result1.second, create_result2.second};
    std::sort(expected_ids.begin(), expected_ids.end());
    std::sort(list_result.second.begin(), list_result.second.end());
    EXPECT_EQ(list_result.second, expected_ids);
}