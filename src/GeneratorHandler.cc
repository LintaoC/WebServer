#include "GeneratorHandler.h"
#include <boost/log/trivial.hpp>
#include <sstream>
#include <iterator>
#include <cstdlib>
#include <array>
#include <random>
#include <boost/beast.hpp>

std::string generate_random_string(size_t length) {
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);
    std::string random_string;
    for (size_t i = 0; i < length; ++i) {
        random_string += CHARACTERS[distribution(generator)];
    }
    return random_string;
}

GeneratorHandler::GeneratorHandler(const std::map<std::string, std::string> &params)
        : description_(""), filename_("") {}

GeneratorHandler::~GeneratorHandler() {}

std::string exec_command(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string parse_form_data(const std::string& body, const std::string& boundary) {
    BOOST_LOG_TRIVIAL(info) << "what is res body: " << body;
    BOOST_LOG_TRIVIAL(info) << "what is res boundary: " << boundary;
    std::string description;
    std::string delimiter = "--" + boundary;
    size_t start = body.find(delimiter) + delimiter.length();
    size_t end = body.find(delimiter, start);
    std::string part = body.substr(start, end - start);
    BOOST_LOG_TRIVIAL(info) << "what is part: " << part;
    std::string desc_field = "Content-Disposition: form-data; name=\"description\"\r\n\r\n";
    size_t pos = part.find(desc_field);
    BOOST_LOG_TRIVIAL(info) << "what is pos: " << pos;
    if (pos != std::string::npos) {
        BOOST_LOG_TRIVIAL(info) << "get into pos: ";
        description = part.substr(pos + desc_field.length(), part.find("\r\n", pos + desc_field.length()) - (pos + desc_field.length()));
        BOOST_LOG_TRIVIAL(info) << "what is description "<<description;
    }

    return description;
}

boost::beast::http::response<boost::beast::http::string_body>
GeneratorHandler::handle_request(const boost::beast::http::request<boost::beast::http::string_body> &req) {
    std::string content_type = req.at(boost::beast::http::field::content_type).to_string();
    std::string boundary = content_type.substr(content_type.find("boundary=") + 9);
    description_ = parse_form_data(req.body(),boundary);
    filename_ = generate_random_string(10);
    BOOST_LOG_TRIVIAL(info) << "Description: " << description_;
    BOOST_LOG_TRIVIAL(info) << "Filename: " << filename_;
    std::string test_output;
    test_output = exec_command("pwd");
    BOOST_LOG_TRIVIAL(info) << "test output: " << test_output;
    std::string list_output;
    list_output = exec_command("ls");
    std::string output_inside_usr;
    output_inside_usr = exec_command("cd usr && ls");
    BOOST_LOG_TRIVIAL(info) << "test usr list output: " << output_inside_usr;
    std::string output_goto_src;
    output_goto_src = exec_command("cd usr/src && ls ");
    BOOST_LOG_TRIVIAL(info) << "test src list output: " << output_goto_src;
    std::string output_goto_project;
    output_goto_project = exec_command("cd usr/src/project && ls ");
    BOOST_LOG_TRIVIAL(info) << "test src/project list output: " << output_goto_project;
    output_goto_project = exec_command("cd usr/src/project/src && ls ");
    BOOST_LOG_TRIVIAL(info) << "test src/project/src list output: " << output_goto_project;
    // Adjust the command to point to the Python script in the parent directory of src
    std::string command = "python3 /usr/src/project/src/generator.py \"" + description_ + "\" \"" + filename_ + "\"";
    BOOST_LOG_TRIVIAL(info) << "Executing command: " << command;

    std::string command_output;
    try {
        command_output = exec_command(command);
    } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(error) << "Command execution failed: " << e.what();
        boost::beast::http::response<boost::beast::http::string_body> res;
        res.result(500);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.set(boost::beast::http::field::access_control_allow_origin, "*");
        res.set(boost::beast::http::field::access_control_allow_methods, "GET, POST, OPTIONS");
        res.set(boost::beast::http::field::access_control_allow_headers, "Content-Type, Authorization");
        res.body() = "<html><body><h1>500 InternalServerError</h1></body></html>";
        res.prepare_payload();
        return res;
    }
    output_goto_src = exec_command("cd usr/src/porject/src/photos && ls ");
    BOOST_LOG_TRIVIAL(info) << "test src list output after calling python: " << output_goto_src;
    BOOST_LOG_TRIVIAL(info) << "Command output: " << command_output;

    boost::beast::http::response<boost::beast::http::string_body> res;
    if (command_output.find("image_url") == std::string::npos) {
        res.result(500);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.set(boost::beast::http::field::access_control_allow_origin, "*");
        res.set(boost::beast::http::field::access_control_allow_methods, "GET, POST, OPTIONS");
        res.set(boost::beast::http::field::access_control_allow_headers, "Content-Type, Authorization");
        res.body() = "<html><body><h1>" + std::to_string(500) + " " + "Get output but blank output" + "</h1></body></html>";
        res.prepare_payload();
        return res;
    }

    // Full path to the generated image
    std::string full_path = "/usr/src/project/src/photos/" + filename_ + ".png";
    BOOST_LOG_TRIVIAL(info) << "The local path is: " << full_path;
    file_.open(full_path, std::ios::binary);
    if (!file_.is_open()) {
        res.result(404);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.set(boost::beast::http::field::access_control_allow_origin, "*");
        res.set(boost::beast::http::field::access_control_allow_methods, "GET, POST, OPTIONS");
        res.set(boost::beast::http::field::access_control_allow_headers, "Content-Type, Authorization");
        res.body() = "<html><body><h1>" + std::to_string(404) + " " + "FileNotFound" + "</h1></body></html>";
        res.prepare_payload();
        return res;
    }

    // Read file into string
    std::string content((std::istreambuf_iterator<char>(file_)), std::istreambuf_iterator<char>());
    file_.close();

    // Prepare HTTP response
    res.result(boost::beast::http::status::ok);
    res.set(boost::beast::http::field::content_type, "image/png");
    res.set(boost::beast::http::field::access_control_allow_origin, "*");
    res.set(boost::beast::http::field::access_control_allow_methods, "GET, POST, OPTIONS");
    res.set(boost::beast::http::field::access_control_allow_headers, "Content-Type, Authorization");
    res.body() = content;
    res.prepare_payload();

    return res;
}