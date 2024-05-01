#ifndef SESSION_FACTORY_H
#define SESSION_FACTORY_H

#include "ISessionFactory.h"
#include "../include/config_parser.h"

class SessionFactory : public ISessionFactory {
public:
    session* create(boost::asio::io_service& io_service, NginxConfig config) override {
        return new session(io_service, config);  // Here we return a new session instance
    }
};

#endif // SESSION_FACTORY_H