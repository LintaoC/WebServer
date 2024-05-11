#include "StaticFileHandler.h"
#include <boost/log/trivial.hpp>
#include <sstream>
#include <iterator>

StaticFileHandler::StaticFileHandler(const std::string& root_path)
        : root_path_(root_path) {}

StaticFileHandler::~StaticFileHandler() {
    if (file_.is_open()) {
        file_.close();
    }
}

boost::beast::http::response<boost::beast::http::string_body>
StaticFileHandler::handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) {
    // Extract only the filename from the request target after the second '/'
    std::string request_target = req.target().to_string();
    std::size_t second_slash_pos = request_target.find('/', 1); // Find the first '/' after the leading '/'
    std::string filename = request_target.substr(second_slash_pos + 1);
    BOOST_LOG_TRIVIAL(info) << "The file path is: " << filename;

    // Full path to the file
    std::string full_path = "../" + root_path_ + "/" + filename;
    BOOST_LOG_TRIVIAL(info) << "The local path is: " << full_path;
    file_.open(full_path, std::ios::binary);

    boost::beast::http::response<boost::beast::http::string_body> res;
    if (!file_.is_open()) {
        res.result(404);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.body() = "<html><body><h1>" + std::to_string(404) + " " + "FileNotFound" + "</h1></body></html>";
        res.prepare_payload();
        return res;
    }

    // Read file into string
    std::string content((std::istreambuf_iterator<char>(file_)), std::istreambuf_iterator<char>());
    file_.close();

    // Prepare HTTP response
    res.result(boost::beast::http::status::ok);
    res.set(boost::beast::http::field::content_type, determineContentType(full_path));
    res.body() = content;
    res.prepare_payload();

    return res;
}

std::string StaticFileHandler::determineContentType(const std::string& path) {
    if (endsWith(path, ".html")) return "text/html";
    if (endsWith(path, ".jpg") || endsWith(path, ".jpeg")) return "image/jpeg";
    if (endsWith(path, ".png")) return "image/png";
    if (endsWith(path, ".txt")) return "text/plain";
    if (endsWith(path, ".zip")) return "application/zip";
    return "application/octet-stream"; // Default MIME type
}

bool StaticFileHandler::endsWith(const std::string& value, const std::string& ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}


