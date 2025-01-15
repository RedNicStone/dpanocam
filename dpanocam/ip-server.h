
#ifndef DPANOCAM_IP_SERVER_H
#define DPANOCAM_IP_SERVER_H

#include "abstract_ip_stream.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <string_view>
#include <optional>
#include <expected>


class ip_server {
    boost::asio::io_context& ioContext;
    boost::asio::ssl::context sslContext;
    boost::asio::ip::tcp::acceptor acceptor;
    bool use_tls = false;

    explicit ip_server(boost::asio::io_context& ioContext);

    public:
    struct keyfiles {
        // certFile:    Public certificate for the server, has to be signed by the authorityCertificateFile
        // keyFile:     Private key for the server
        // caFile:      Certificate for the CA that signed the certificateFile

        std::string certFile;
        std::string keyFile;
        std::string caFile;
    };

    static std::expected<ip_server, std::string> create(boost::asio::io_context& ioContext, const std::optional<keyfiles>& keys = std::nullopt);
    ~ip_server();

    boost::asio::io_context& get_io_context() { return ioContext; }

    ip_server(const ip_server&) = delete;
    ip_server& operator=(const ip_server&) = delete;

    ip_server(ip_server&& other) noexcept = default;

    std::expected<void, std::string> bind(uint32_t port, std::string_view interface = "0.0.0.0");

    std::expected<std::unique_ptr<abstract_stream>, std::string> awaitConnection();
};


#endif //DPANOCAM_IP_SERVER_H
