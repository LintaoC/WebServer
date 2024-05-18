#include "CRUDHandler.h"

#include <utility>
#include <boost/beast/http.hpp>

CRUDHandler::CRUDHandler(std::shared_ptr<EntityDatabase> entity_database) :
        entity_database_(std::move(entity_database)) {
}

CRUDHandler::~CRUDHandler() = default;

Response CRUDHandler::handle_request(const Request &req) {
    // TODO: handle dispatching of requests based on the HTTP method
    Response res;
    res.result(boost::beast::http::status::not_implemented);
    return res;
}

Response CRUDHandler::handle_post(const Request &req) {
    // TODO: POST request handling
    Response res;
    res.result(boost::beast::http::status::not_implemented);
    return res;
}

Response CRUDHandler::handle_get(const Request &req) {
    // TODO: GET request handling
    Response res;
    res.result(boost::beast::http::status::not_implemented);
    return res;
}

Response CRUDHandler::handle_put(const Request &req) {
    // TODO: PUT request handling
    Response res;
    res.result(boost::beast::http::status::not_implemented);
    return res;
}

Response CRUDHandler::handle_delete(const Request &req) {
    // TODO: DELETE request handling
    Response res;
    res.result(boost::beast::http::status::not_implemented);
    return res;
}

