#ifndef SESSION_FACTORY_H
#define SESSION_FACTORY_H

#include "ISessionFactory.h"
#include "../include/config_parser.h"
#include "RequestHandlerFactory.h"
#include <map>
#include <boost/shared_ptr.hpp>

class SessionFactory : public ISessionFactory {
public:
    boost::shared_ptr<session> create(boost::asio::io_service& io_service, std::map<std::string, RequestHandlerFactory*>* routes) override {
        return boost::make_shared<session>(io_service, routes);  // Here we return a new session instance
    }
};

#endif // SESSION_FACTORY_H