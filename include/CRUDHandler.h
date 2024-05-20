#ifndef API_HANDLER_H
#define API_HANDLER_H

#include <boost/log/trivial.hpp>

#include "RequestHandler.h"
#include "EntityDatabase.h"

#define Request boost::beast::http::request<boost::beast::http::string_body>
#define Response boost::beast::http::response<boost::beast::http::string_body>

/**
 * A handler that handles CRUD operations on entities.
 */
class CRUDHandler : public RequestHandler {
public:
    explicit CRUDHandler(const std::map<std::string, std::string>& params);

    ~CRUDHandler() override;

    /// Overridden method to handle requests
    Response handle_request(const Request &req) override;

private:
    /// Handles a POST request
    Response handle_post(const Request &req);

    /// Handles a GET request
    Response handle_get(const Request &req);

    /// Handles a PUT request
    Response handle_put(const Request &req);

    /// Handles a DELETE request
    Response handle_delete(const Request &req);

    /// The root path for the entities
    const std::string root_path_;

    /// The database of entities
    std::shared_ptr<EntityDatabase> entity_database_;

    /// Extracts the parts of a path, remove /api/
    std::vector<std::string> extract_path_parts(const std::string &target);
};

#endif //API_HANDLER_H
