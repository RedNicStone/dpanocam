
#ifndef DPANOCAM_IP_SERVER_H
#define DPANOCAM_IP_SERVER_H

#include "../dpanocam/abstract_ip_stream.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <string_view>
#include <optional>
#include <expected>


class ip_client {
    boost::asio::io_context& ioContext;
    boost::asio::ssl::context sslContext;
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoints;
    bool use_tls = false;

    explicit ip_client(boost::asio::io_context& ioContext);

public:
    struct keyfiles {
        // certFile:    Public certificate for the client
        // keyFile:     Private key for the client
        // caFile:      Certificate for the CA that signed the server's certificate

        std::string certFile;
        std::string keyFile;
        std::string caFile;
    };

    static std::expected<ip_client, std::string> create(boost::asio::io_context& ioContext, const std::optional<keyfiles>& keys = std::nullopt);
    ~ip_client();

    ip_client(const ip_client&) = delete;
    ip_client& operator=(const ip_client&) = delete;

    ip_client(ip_client&& other) noexcept = default;

    std::expected<void, std::string> bind(uint32_t port, std::string_view host);
    std::expected<std::unique_ptr<abstract_stream>, std::string> openConnection();
};


#endif //DPANOCAM_IP_SERVER_H
