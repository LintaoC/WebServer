#include "RequestHandlerFactory.h"
#include <boost/log/trivial.hpp>
#include <boost/log/attributes.hpp>
#include "EchoHandler.h"
#include "RequestHandler.h"
#include "StaticFileHandler.h"
#include "NotFoundHandler.h"

// Constructor implementation
RequestHandlerFactory::RequestHandlerFactory(const std::string& type, const std::string& path)
        : handlerType(type), rootPath(path), relativePath("") {
    BOOST_LOG_TRIVIAL(info) <<"Request Handler Factory created with type of "<<type<<" and root path of "<<path;
}


 //Getter implementations (optional, uncomment if needed)

std::string RequestHandlerFactory::getHandlerType() const {
    return handlerType;
}

std::string RequestHandlerFactory::getRootPath() const {
    return rootPath;
}

std::string RequestHandlerFactory::getRelativePath() const {
    return relativePath;
}

void RequestHandlerFactory::setRelativePath(std::string rp){
    relativePath = rp;
}

RequestHandler* RequestHandlerFactory::buildRequestHandler() const {
    if (handlerType == "EchoHandler"){
        return new EchoHandler();
    } else if(handlerType == "StaticHandler"){
        return new StaticFileHandler(rootPath, relativePath);
    } else {
        return new NotFoundHandler();
    }
}



