#include "RequestHandlerFactory.h"
#include <boost/log/trivial.hpp>
#include <boost/log/attributes.hpp>

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


 //Additional method implementations can be added here
