#include "CRUDHandler.h"

#include <utility>
#include <boost/beast/http.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>

namespace http = boost::beast::http;

CRUDHandler::CRUDHandler(const std::map<std::string, std::string>& params) :
        root_path_(params.at("root")) {
}

CRUDHandler::~CRUDHandler() = default;

Response CRUDHandler::handle_request(const Request &req) {
    std::string full_path = "../" + root_path_ ;
    BOOST_LOG_TRIVIAL(info) << "The full path for current DB is: " << full_path;
    entity_database_ = std::make_shared<EntityDatabase>(full_path);

    switch (req.method()) {
        case boost::beast::http::verb::post:
            return handle_post(req);
        case boost::beast::http::verb::get:
            return handle_get(req);
        case boost::beast::http::verb::put:
            return handle_put(req);
        case boost::beast::http::verb::delete_:
            return handle_delete(req);
        default:
            // Send 405 Method Not Allowed for unsupported methods
            Response res;
            res.result(boost::beast::http::status::method_not_allowed);
            res.body() = "Method not allowed";
            res.prepare_payload();
            BOOST_LOG_TRIVIAL(info) << "Method not allowed for " << std::string(req.target());
            return res;
    }
}

std::vector<std::string> CRUDHandler::extract_path_parts(const std::string &target) {
    std::string path = target.substr(5); // Remove "/api/"
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string item;
    while (std::getline(ss, item, '/')) {
        if (!item.empty()) {
            parts.push_back(item);
        }
    }
    return parts;
}

// POST request handling
Response CRUDHandler::handle_post(const Request &req) {
    BOOST_LOG_TRIVIAL(info) << "Handling POST request";
    std::vector<std::string> parts = extract_path_parts(std::string(req.target()));
    Response res;

    if (parts.size() == 1) {
        std::string entity_name = parts[0];
        std::stringstream ss;
        ss << req.body();
        
        // Validate JSON
        try {
            boost::property_tree::ptree pt;
            boost::property_tree::read_json(ss, pt);

            // Create the entity and get the id
            auto [status, id] = entity_database_->create_entity(entity_name);

            if (status == EntityDatabase::Success) {
                // Write the entire JSON body to the entity
                auto write_status = entity_database_->write_to_entity(entity_name, id, req.body());

                if (write_status == EntityDatabase::Success) {
                    res.result(http::status::created);
                    res.set(http::field::content_type, "application/json");
                    boost::property_tree::ptree response_pt;
                    response_pt.put("id", id);
                    std::ostringstream oss;
                    boost::property_tree::write_json(oss, response_pt);
                    res.body() = oss.str();
                } else {
                    res.result(http::status::internal_server_error);
                    res.body() = "Failed to write content to entity";
                    BOOST_LOG_TRIVIAL(error) << "Failed to write content to entity " << id;
                }
            } else {
                res.result(http::status::internal_server_error);
                res.body() = "Failed to create entity";
                BOOST_LOG_TRIVIAL(error) << "Failed to create entity for " << req.target();
            }
        } catch (const boost::property_tree::json_parser_error& e) {
            res.result(boost::beast::http::status::bad_request);
            res.body() = "Invalid JSON format";
            BOOST_LOG_TRIVIAL(error) << "JSON parsing error: " << e.what();
        }
    } else {
        res.result(boost::beast::http::status::bad_request);
        res.body() = "Invalid request path";
        BOOST_LOG_TRIVIAL(info) << "Invalid request path: " << req.target();
    }

    res.prepare_payload();
    BOOST_LOG_TRIVIAL(info) << "POST request handled";
    return res;
}


// GET request handling
Response CRUDHandler::handle_get(const Request &req) {
    std::string target = std::string(req.target());
    std::vector<std::string> parts = extract_path_parts(target);

    Response res;
    if (parts.size() == 1) {
        // List entities
        std::string entity_name = parts[0];
        auto [status, ids] = entity_database_->list_entities(entity_name);
        if (status == EntityDatabase::Success) {
            // List entities
            std::string entity_name = parts[0];
            auto [status, ids] = entity_database_->list_entities(entity_name);
            if (status == EntityDatabase::Success) {
                res.result(boost::beast::http::status::ok);
                res.set(boost::beast::http::field::content_type, "application/json");

                // Construct the JSON array directly
                std::ostringstream oss;
                oss << "[";
                for (size_t i = 0; i < ids.size(); ++i) {
                    oss << "\"" << ids[i] << "\"";
                    if (i != ids.size() - 1) {
                        oss << ",";
                    }
                }
                oss << "]";

                res.body() = oss.str();
                BOOST_LOG_TRIVIAL(info) << "Listed entities for " << entity_name;
            }
        } else {
            res.result(boost::beast::http::status::not_found);
            res.body() = "Entity type not found";
            BOOST_LOG_TRIVIAL(info) << "Entity type not found: " << entity_name;
        }
    } else if (parts.size() == 2) {
        // Get entity by ID
        std::string entity_name = parts[0];
        std::string entity_id = parts[1];
        auto [status, content] = entity_database_->read_from_entity(entity_name, entity_id);
        if (status == EntityDatabase::Success) {
            res.result(boost::beast::http::status::ok);
            res.set(boost::beast::http::field::content_type, "application/json");
            res.body() = content;
            BOOST_LOG_TRIVIAL(info) << "Read entity " << entity_id << " for " << entity_name;
        } else {
            res.result(boost::beast::http::status::not_found);
            res.body() = "Entity not found";
            BOOST_LOG_TRIVIAL(info) << "Entity not found: " << entity_id;
        }
    } else {
        res.result(boost::beast::http::status::bad_request);
        res.body() = "Invalid request path";
        BOOST_LOG_TRIVIAL(info) << "Invalid request path: " << target;
    }
    res.prepare_payload();
    return res;
}

// PUT request handling
Response CRUDHandler::handle_put(const Request &req) {
    std::string target = std::string(req.target());
    std::vector<std::string> parts = extract_path_parts(target);

    Response res;
    if (parts.size() == 2) {
        std::string entity_name = parts[0];
        std::string entity_id = parts[1];
        std::stringstream body_ss;
        body_ss << req.body();

        // Validate JSON
        try{
            boost::property_tree::ptree pt;
            boost::property_tree::read_json(body_ss, pt);

            std::string content = req.body(); // Use the entire body as content
            auto status = entity_database_->write_to_entity(entity_name, entity_id, content);

            if (status == EntityDatabase::Success) {
                res.result(boost::beast::http::status::ok);
                res.body() = "Entity updated successfully";
                BOOST_LOG_TRIVIAL(info) << "Entity updated: " << entity_id << " for " << entity_name;
            } else {
                res.result(boost::beast::http::status::not_found);
                res.body() = "Entity not found";
                BOOST_LOG_TRIVIAL(info) << "Entity not found: " << entity_id;
            }
        } catch (const boost::property_tree::json_parser_error& e) {
            res.result(boost::beast::http::status::bad_request);
            res.body() = "Invalid JSON format";
            BOOST_LOG_TRIVIAL(error) << "JSON parsing error: " << e.what();
            return res;
        }
    } else {
        res.result(boost::beast::http::status::bad_request);
        res.body() = "Invalid request path";
        BOOST_LOG_TRIVIAL(info) << "Invalid request path: " << target;
    }
    res.prepare_payload();
    return res;
}

// DELETE request handling
Response CRUDHandler::handle_delete(const Request &req) {
    std::string target = std::string(req.target());
    std::vector<std::string> parts = extract_path_parts(target);

    Response res;
    if (parts.size() == 2) {
        std::string entity_name = parts[0];
        std::string entity_id = parts[1];
        auto status = entity_database_->delete_entity(entity_name, entity_id);

        if (status == EntityDatabase::Success) {
            res.result(boost::beast::http::status::ok);
            res.body() = "Entity deleted successfully";
            BOOST_LOG_TRIVIAL(info) << "Entity deleted: " << entity_id << " for " << entity_name;
        } else {
            res.result(boost::beast::http::status::not_found);
            res.body() = "Entity not found";
            BOOST_LOG_TRIVIAL(info) << "Entity not found: " << entity_id;
        }
    } else {
        res.result(boost::beast::http::status::bad_request);
        res.body() = "Invalid request path";
        BOOST_LOG_TRIVIAL(info) << "Invalid request path: " << target;
    }
    res.prepare_payload();
    return res;
}

