# Source Code Lay Out

### Server Initialization:

`server_main.cc`:\
Initializes logging.\
Parses the configuration file using `NginxConfigParser`.\
Creates a `server` instance and starts it.


### Configuration Parsing:

`config_parser.cc`:\
`NginxConfigParser`: Parses the configuration file and populates `NginxConfig`.\
`NginxConfig`: Stores the parsed configuration, including mappings of URL paths to handler types.

### Request Handling:

`RequestHandlerFactory.cc`:\
`RequestHandlerFactory`: Creates specific request handlers (`EchoHandler`, `StaticFileHandler`, `NotFoundHandler`) based on configuration.\
`RequestHandler`: Base class for all request handlers.

`EchoHandler.cc`:\
`EchoHandler`: Inherits from `RequestHandler` and handles echoing the received request.

`StaticFileHandler.cc`:\
`StaticFileHandler`: Inherits from `RequestHandle`r and serves static files.

`NotFoundHandler.cc`:\
`NotFoundHandler`: Inherits from `RequestHandler` and returns a 404 Not Found response.

### Server Operation:

`server.cc`:\
Accepts incoming connections and use factory to create `session` instances.

`session.cc`: \
Manages individual client connections, reads requests, and delegates them to appropriate request handlers.\
Uses `RequestHandlerFactory` to obtain the appropriate request handler for each request.

# Build & Run

## Start Docker

`tools/env/start.sh -u ${USER}` 

## Run Docker
`docker build -f docker/base.Dockerfile -t f004:base .`\
`docker build -f docker/Dockerfile -t my_image .`\
`docker run --rm -p 8080:8080 --name my_run my_image:latest`

## Create and go to build directory under f004

`mkdir build` \
`cd build`

## Start compiling

`cmake ..`\
`make` 

## Start the server

`bin/server ../config` 

## Kill a server (if needed)

`sudo lsof -i :8080` \
`sudo kill -9 PID` 

## Run the program

### After starting a server listen on port 8080:
#### Sending echo request to the server:

`echo -ne "GET /echo1/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" | nc localhost 8080`

#### Sending static file request to the server:

`echo -ne "GET /static/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" | nc localhost 8080`


# Test

## Unit test, Intergration test & Coverage report

### Build and go to build_coverage directory under f004
`mkdir build_coverage`\
`cd build_coverage`

### Start compiling and generate test coverage report
`cmake -DCMAKE_BUILD_TYPE=Coverage ..`\
`make coverage`

### Debug testing
`ctest --verbose`

## Intergration test only

### Go to integration_tests directory under f004
`cd integration_tests`\
`./integration_tests.sh`

# Building Container in Google Cloud

`gcloud builds submit --config docker/cloudbuild.yaml .`

# Adding a Request Handler

### 1. Create a new class (both .h and .cc files) that inherits from `RequestHandler` and implements the `handle_request` method.

#### For example, adding an `ExampleHandler`:

ExampleHandler.h
```
#include "RequestHandler.h"

class ExampleHandler : public RequestHandler {
public:
    ExampleHandler();
    ~ExampleHandler();
    boost::beast::http::response<boost::beast::http::string_body> handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) override;
};
```

ExampleHandler.cc

```
#include "ExampleHandler.h"

ExampleHandler::ExampleHandler() {
}

ExampleHandler::~ExampleHandler() {
}

boost::beast::http::response<boost::beast::http::string_body> ExampleHandler::handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) {
    boost::beast::http::response<boost::beast::http::string_body> res;
    res.result(boost::beast::http::status::ok);
    res.set(boost::beast::http::field::content_type, "text/plain");
    res.body() = "Example Handler Response";
    res.prepare_payload();
    return res;
}
```

### 2. Update `RequestHandlerFactory` to include the new handler type.

RequestHandlerFactory.cc

```
#include "ExampleHandler.h"

RequestHandler* RequestHandlerFactory::buildRequestHandler() const {
    if (handlerType == "EchoHandler") {
        return new EchoHandler();
    } else if (handlerType == "StaticHandler") {
        return new StaticFileHandler(rootPath, relativePath);
    } else if (handlerType == "ExampleHandler") {
        return new ExampleHandler();
    } else {
        return new NotFoundHandler();
    }
}
```

### 3. Update Configuration

```
location /example ExampleHandler {
    root ./path;
}
```

### 4. Update header files wherever needed

## Well-documented example of an existing handler

EchoHandler.cc

```
#include "EchoHandler.h"

EchoHandler::EchoHandler() {
}

EchoHandler::~EchoHandler() {
}

// Implementation of handle_request for EchoHandler
boost::beast::http::response<boost::beast::http::string_body>
EchoHandler::handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) {
    boost::beast::http::response<boost::beast::http::string_body> res;

    // Set the status code to 200 OK
    res.result(boost::beast::http::status::ok);

    // Create a string stream to build the full request text
    std::stringstream ss;

    // Serialize the request base (method, target, version, headers)
    ss << req.method_string() << " " << req.target() << " HTTP/" << (req.version() / 10) << "." << (req.version() % 10) << "\r\n";
    for (const auto& field : req.base()) {
        ss << field.name_string() << ": " << field.value() << "\r\n";
    }

    // Append the request body
    ss << "\r\n" << req.body();

    // Set the response body to the entire formatted request
    res.set(boost::beast::http::field::content_type, "text/plain");
    res.body() = ss.str();

    // Prepare the response
    res.prepare_payload();

    return res;
}
```

## Well-documented header file

EchoHandler.h

```
#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include "RequestHandler.h"
#include <boost/beast/http.hpp>
#include <string>

// EchoHandler class that echoes back the request
class EchoHandler : public RequestHandler {
public:
    // Constructor
    EchoHandler();
    // Destructor
    ~EchoHandler();

    // Overridden method to handle requests
    boost::beast::http::response<boost::beast::http::string_body>
    handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) override;
};

#endif // ECHO_HANDLER_H
```