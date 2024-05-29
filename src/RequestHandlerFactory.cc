#include "RequestHandlerFactory.h"
#include <boost/log/trivial.hpp>
#include <boost/log/attributes.hpp>
#include <utility>
#include "EchoHandler.h"
#include "RequestHandler.h"
#include "StaticFileHandler.h"
#include "NotFoundHandler.h"
#include "SleepRequestHandler.h"
#include "CRUDHandler.h"
#include "EntityDatabase.h"
#include "HealthHandler.h"

// Constructor implementation
RequestHandlerFactory::RequestHandlerFactory(const std::string &type,
                                             const std::map<std::string, std::string> &handler_params)
        : handlerType(type), handlerParams(handler_params) {
    BOOST_LOG_TRIVIAL(info) << "Request Handler Factory created with type of "
                            << type
                            << " and handler params of size "
                            << handler_params.size();
}


//Getter implementations (optional, uncomment if needed)

std::string RequestHandlerFactory::getHandlerType() const {
    return handlerType;
}

std::string RequestHandlerFactory::getRelativePath() const {
    return handlerParams.at("relative_path");
}

void RequestHandlerFactory::setRelativePath(std::string rp) {
    handlerParams["relative_path"] = std::move(rp);
}

std::map<std::string, std::string> RequestHandlerFactory::getHandlerParams() const {
    return handlerParams;
}

RequestHandler *RequestHandlerFactory::buildRequestHandler() const {
    if (handlerType == "EchoHandler") {
        return new EchoHandler();
    } else if (handlerType == "StaticHandler") {
        return new StaticFileHandler(handlerParams);
    } else if (handlerType == "SleepHandler") {
        return new SleepRequestHandler();
    } else if (handlerType == "CRUDHandler"){
        std::string full_path = "../" + handlerParams.at("root") ;
        std::shared_ptr<EntityDatabase> entityDatabase = std::make_shared<EntityDatabase>(full_path);
        return new CRUDHandler (entityDatabase);
    } 
    else if (handlerType == "HealthHandler") {
        return new HealthHandler();
    }
    else {
        return new NotFoundHandler();
    }
}
