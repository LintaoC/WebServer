#include "RequestHandlerFactory.h"
#include <boost/log/trivial.hpp>
#include <boost/log/attributes.hpp>
#include "EchoHandler.h"
#include "RequestHandler.h"
#include "StaticFileHandler.h"
#include "NotFoundHandler.h"

// Constructor implementation
RequestHandlerFactory::RequestHandlerFactory(const std::string& type, const std::string& path)
        : handlerType(type), rootPath(path) {
    BOOST_LOG_TRIVIAL(info) <<"Request Handler Factory created with type of "<<type<<" and root path of "<<path;
}


 //Getter implementations (optional, uncomment if needed)

std::string RequestHandlerFactory::getHandlerType() const {
    return handlerType;
}

std::string RequestHandlerFactory::getRootPath() const {
    return rootPath;
}

RequestHandler* RequestHandlerFactory::buildRequestHandler() const {
    if (handlerType == "EchoHandler"){
        return new EchoHandler();
    } else if(handlerType == "StaticHandler"){
        return new StaticFileHandler(rootPath);
    } else {
        return new NotFoundHandler();
    }
}



