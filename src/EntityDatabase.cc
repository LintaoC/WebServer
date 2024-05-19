#include "EntityDatabase.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace fs = std::filesystem;
namespace uuids = boost::uuids;

/**
 * Constructor that initializes the EntityDatabase with a given root path.
 * It ensures that the root directory exists or creates it if it doesn't.
 * 
 * @param path The root directory path where entity data will be stored.
 */
EntityDatabase::EntityDatabase(const std::string &path) 
    : data_path(path) {

    fs::path root_dir(data_path);
    if (!fs::exists(root_dir)) {
        fs::create_directories(root_dir);
    }
}

/**
 * Creates a new entity in the database. This method creates a new file for storing the entity data. It returns the ID of the
 * newly created entity.
 * 
 * @param entity_name The name of the entity type under which to store the data.
 * @return A pair containing a StatusCode and the new entity ID.
 */
std::pair<EntityDatabase::StatusCode, std::string> EntityDatabase::create_entity(const std::string &entity_name) {
    fs::path entity_dir(data_path + "/" + entity_name);

    // Create a directory for a new entity type if it does not exist
    if (!fs::exists(entity_dir)) {
        fs::create_directories(entity_dir);  
    }

    // Generate a new UUID for the entity ID
    uuids::uuid uuid = uuids::random_generator()();
    std::string new_id = to_string(uuid);

    // Create a new file for the entity
    fs::path filepath = entity_dir / new_id;
    std::ofstream new_file(filepath);
    if (!new_file) {
        return { FileSystemError, "" };  // Return an error if the file cannot be opened
    }
    new_file.close();
    return { Success, new_id };
}

/**
 * Deletes an entity from the database by removing its file.
 * 
 * @param entity_name The type name of the entity.
 * @param entity_id The ID of the entity to delete.
 * @return StatusCode indicating the result of the operation.
 */
EntityDatabase::StatusCode EntityDatabase::delete_entity(const std::string &entity_name, const std::string &entity_id) {
    fs::path entity_dir(data_path + "/" + entity_name);
    
    // Check if the directory for the entity type exists
    if (!fs::exists(entity_dir)) {
        return EntityTypeNotFound;  // Return error if the directory does not exist
    }
    
    fs::path filepath = entity_dir / entity_id;
    
    // Check if the file for the specific entity ID exists
    if (!fs::exists(filepath)) {
        return EntityIDNotFound;  // Return error if the file does not exist
    }
    
    // Remove the file
    try {
        fs::remove(filepath);
    } catch (const fs::filesystem_error& e) {
        return FileSystemError; // Return error if file removal fails
    }
    
    return Success;
}


/**
 * Updates (writes) new content to an entity's file.
 * 
 * @param entity_name The type name of the entity.
 * @param entity_id The ID of the entity to update.
 * @param content The new content to write to the entity.
 * @return StatusCode indicating the result of the operation.
 */
EntityDatabase::StatusCode EntityDatabase::write_to_entity(const std::string &entity_name, const std::string &entity_id, const std::string &content) {
    fs::path entity_dir(data_path + "/" + entity_name);
    
    // Check if the directory for the entity type exists
    if (!fs::exists(entity_dir)) {
        return EntityTypeNotFound;  // Return error if the directory does not exist
    }
    fs::path filepath = entity_dir / entity_id;
    
    // Check if the file for the specific entity ID exists
    if (!fs::exists(filepath)) {
        return EntityIDNotFound;  // Return error if the file does not exist
    }
    
    // Write the new content to the file
    std::ofstream file(filepath);
    if (!file) {
        return FileSystemError;  // Ensure the file is openable
    }
    file << content;  // Write content to the file
    return Success;
}

/**
 * Reads content from an entity's file.
 * 
 * @param entity_name The type name of the entity.
 * @param entity_id The ID of the entity to read.
 * @return A pair containing a StatusCode and the content stored in the entity.
 */
std::pair<EntityDatabase::StatusCode, std::string> EntityDatabase::read_from_entity(const std::string &entity_name, const std::string &entity_id) const {
    fs::path entity_dir(data_path + "/" + entity_name);
    
    // Check if the directory for the entity type exists
    if (!fs::exists(entity_dir)) {
        return { EntityTypeNotFound, "" };  // Return error if the directory does not exist
    }
    fs::path filepath = entity_dir / entity_id;
    
    // Check if the file for the specific entity ID exists
    if (!fs::exists(filepath)) {
        return { EntityIDNotFound, "" };  // Return error if the file does not exist
    }

    // Read the content from the file
    std::ifstream file(filepath);
    if (!file) {
        return {FileSystemError, ""};  // Return error if the file cannot be opened
    }
    std::stringstream buffer;
    buffer << file.rdbuf();  // Read the file content into a string
    return {Success, buffer.str()};
}

/**
 * Lists all entity IDs for a given type.
 * 
 * @param entity_name The type name of the entity to list IDs for.
 * @return A pair containing a StatusCode and a vector of entity IDs.
 */
std::pair<EntityDatabase::StatusCode, std::vector<std::string>> EntityDatabase::list_entities(const std::string &entity_name) const {
    fs::path entity_dir(data_path + "/" + entity_name);
    
    // Check if the directory exists
    if (!fs::exists(entity_dir)) {
        return {EntityTypeNotFound, {}};  // Return error if the directory does not exist
    }
    
    // Collect the IDs of all entities in the directory
    std::vector<std::string> ids;
    
    try {
        // Iterate through the directory and collect filenames as entity IDs
        for (const auto& entry : fs::directory_iterator(entity_dir)) {
            ids.push_back(entry.path().filename().string());
        }
    } catch (const fs::filesystem_error& e) {
        return {FileSystemError, {}};  // Return error if directory iteration fails
    }
    
    return {Success, ids};  // Return success with the list of IDs
}