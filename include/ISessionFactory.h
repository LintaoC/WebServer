// ISessionFactory.h
#ifndef ISESSION_FACTORY_H
#define ISESSION_FACTORY_H

#include "session.h"
#include "../include/config_parser.h"

class ISessionFactory {
public:
    virtual ~ISessionFactory() {}
    virtual session* create(boost::asio::io_service& io_service, NginxConfig config) = 0;
};

#endif