#ifndef REQUEST_HANDLER_FACTORY_H
#define REQUEST_HANDLER_FACTORY_H

#include <string>
#include "RequestHandler.h"

class RequestHandlerFactory {
private:
    std::string handlerType;
    std::string rootPath;

public:
    // Constructor with parameters for handler type and root path
    RequestHandlerFactory(const std::string& type, const std::string& path);

    // Getters
    std::string getHandlerType() const;
    std::string getRootPath() const;

    RequestHandler* buildRequestHandler() const;

};

#endif // REQUEST_HANDLER_FACTORY_H
