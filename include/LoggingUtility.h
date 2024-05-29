#ifndef LOGGING_UTILITY_H
#define LOGGING_UTILITY_H

#include <boost/log/trivial.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>

inline void log_response_metrics(const boost::beast::http::request<boost::beast::http::string_body>& req,
                                 const boost::beast::http::response<boost::beast::http::string_body>& res,
                                 const std::string& handler_name,
                                 const boost::asio::ip::tcp::endpoint& remote_endpoint) {
    BOOST_LOG_TRIVIAL(info) << "[ResponseMetrics] "
                            << "ResponseCode=" << res.result_int() << " "
                            << "RequestPath=" << std::string(req.target()) << " "
                            << "RequestIP=" << remote_endpoint.address().to_string() << ":" << remote_endpoint.port() << " "
                            << "Handler=" << handler_name << " ";
}

#endif // LOGGING_UTILITY_H
