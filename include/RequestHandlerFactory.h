#ifndef REQUEST_HANDLER_FACTORY_H
#define REQUEST_HANDLER_FACTORY_H

#include <string>
#include <map>
#include "RequestHandler.h"

class RequestHandlerFactory {
private:
    std::string handlerType;

    /// Stores the constructing parameters for the handler as name-value pairs
    /// Note: the relative path is stored within with key "relative_path"
    std::map<std::string, std::string> handlerParams;

public:
    // Constructor with parameters for handler type and root path
    RequestHandlerFactory(const std::string& type, const std::map<std::string, std::string>& handler_params);

    // Getters
    [[nodiscard]] std::string getHandlerType() const;
    std::string getRelativePath() const;
    void setRelativePath(std::string rp);
    [[nodiscard]] std::map<std::string, std::string> getHandlerParams() const;

    virtual RequestHandler* buildRequestHandler() const;

};

#endif // REQUEST_HANDLER_FACTORY_H
