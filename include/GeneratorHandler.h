#ifndef GENERATOR_HANDLER_H
#define GENERATOR_HANDLER_H

#include "RequestHandler.h"
#include <string>
#include <map>
#include <fstream>
#include <boost/beast/http.hpp>

class GeneratorHandler : public RequestHandler {
public:
    GeneratorHandler(const std::map<std::string, std::string> &params);
    ~GeneratorHandler();

    boost::beast::http::response<boost::beast::http::string_body>
    handle_request(const boost::beast::http::request<boost::beast::http::string_body> &req) override;

private:
    std::string description_;
    std::string filename_;
    std::ifstream file_;
};

#endif // GENERATOR_HANDLER_H
