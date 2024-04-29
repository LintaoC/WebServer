#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <sstream>

class RequestHandler {
public:
    virtual ~RequestHandler() {}
    virtual void handleRequest(const std::string& request_data) = 0;
};

#endif // REQUEST_HANDLER_H