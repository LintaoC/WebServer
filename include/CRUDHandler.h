#ifndef API_HANDLER_H
#define API_HANDLER_H

#include "RequestHandler.h"
#include "EntityDatabase.h"

#define Request boost::beast::http::request<boost::beast::http::string_body>
#define Response boost::beast::http::response<boost::beast::http::string_body>

/**
 * A handler that handles CRUD operations on entities.
 */
class CRUDHandler : public RequestHandler {
public:
    explicit CRUDHandler(std::shared_ptr<EntityDatabase> entity_database);

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

    std::shared_ptr<EntityDatabase> entity_database_;
};

#endif //API_HANDLER_H
