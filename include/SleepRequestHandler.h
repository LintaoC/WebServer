#include "RequestHandler.h"

class SleepRequestHandler : public RequestHandler {
public:
    SleepRequestHandler();
    virtual ~SleepRequestHandler();
    boost::beast::http::response<boost::beast::http::string_body> handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req) override;
};
