// ISessionFactory.h
#ifndef ISESSION_FACTORY_H
#define ISESSION_FACTORY_H

#include "session.h"
#include "../include/config_parser.h"
#include "RequestHandlerFactory.h"
#include <map>
#include <boost/shared_ptr.hpp>

class ISessionFactory {
public:
    virtual ~ISessionFactory() {}
    virtual boost::shared_ptr<session> create(boost::asio::io_service& io_service, std::map<std::string, RequestHandlerFactory*>* routes) = 0;
};

#endif