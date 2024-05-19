#ifndef ENTITY_DATABASE_H
#define ENTITY_DATABASE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

/**
 * A simple entity storage database.
 */
class EntityDatabase {
public:
    /**
     * Create a new Entity database with a given file system root path to store
     * all data files.
     */
    explicit EntityDatabase(const std::string &data_path);

    /// Database operations status
    enum StatusCode {
        Success,
        EntityTypeNotFound,
        EntityIDNotFound,
        FileSystemError,
    };

    /**
     * Create a new Entity object in the database. If the entity type doesn't
     * exist, it will be created.
     *
     * @param entity_name The type name of the Entity to be created.
     * @return A pair of the operation status, and the id of the newly created
     *         Entity object.
     * @note Possible StatusCode to return: Success, FileSystemError.
     */
    std::pair<StatusCode, std::string> create_entity(const std::string &entity_name);

    /**
     * Delete an Entity object with the given name of given id.
     *
     * @param entity_name The type name of the Entity object to delete from.
     * @param entity_id The id of the Entity to be deleted.
     * @return The operation status.
     * @note Possible StatusCode to return: Success, EntityTypeNotFound,
     *       EntityIDNotFound, FileSystemError.
     */
    StatusCode delete_entity(const std::string &entity_name, const std::string &entity_id);

    /**
     * Write content to the specified entity object
     *
     * @param entity_name The type name of the Entity object.
     * @param entity_id The id of the Entity to write to.
     * @param content The content to write to the Entity.
     * @return The operation status.
     * @note Possible StatusCode to return: Success, EntityTypeNotFound,
     *       EntityIDNotFound, FileSystemError.
     */
    StatusCode write_to_entity(const std::string &entity_name, const std::string &entity_id, const std::string &content);

    /**
     * Read content stored in an entity object
     *
     * @param entity_name The type name of the Entity object.
     * @param entity_id The id of the Entity to read from.
     * @return A pair of operation status, and the content stored in the object.
     * @note Possible StatusCode to return: Success, EntityTypeNotFound,
     *       EntityIDNotFound, FileSystemError.
     */
    std::pair<StatusCode, std::string> read_from_entity(const std::string &entity_name, const std::string &entity_id) const;

    /**
     * Get the ids of all existing Entity object with a given type name.
     *
     * @attention If the Entity type with the give name doesn't exist, an
     *            EntityTypeNotFound status will be returned. However, if the Entity
     *            type exists but no objects are stored under this type, a Success
     *            status with an empty vector will be returned.
     *
     * @param entity_name The type name of the entity.
     * @return A pair of operation status, and a list of Entity ids stored under the
     *         specified type name in the database.
     * @note Possible StatusCode to return: Success, EntityTypeNotFound,
     *       FileSystemError.
     */
    std::pair<StatusCode, std::vector<std::string>> list_entities(const std::string &entity_name) const;

private:
    /// The root path to store all Entity files.
    std::string data_path;
};

#endif //ENTITY_DATABASE_H
