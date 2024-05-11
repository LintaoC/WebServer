#ifndef REQUEST_HANDLER_FACTORY_H
#define REQUEST_HANDLER_FACTORY_H

#include <string>

class RequestHandlerFactory {
private:
    std::string handlerType;
    std::string rootPath;

public:
    // Constructor with parameters for handler type and root path
    RequestHandlerFactory(const std::string& type, const std::string& path);

    // Getters (optional, add if needed)
    std::string getHandlerType() const;
    std::string getRootPath() const;

    // Additional methods can be added here
};

#endif // REQUEST_HANDLER_FACTORY_H
