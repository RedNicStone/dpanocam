
#include "ip-server.h"

#include <filesystem>


namespace asio = boost::asio;
using asio::ip::tcp;

asio::ssl::context::file_format determineFileFormat(std::string_view fileType) {
    std::filesystem::path path(fileType);
    const auto& extension = path.extension();
    if (fileType == "asn1")
        return asio::ssl::context::asn1;
    else  // By default, assume PEM
        return asio::ssl::context::pem;
}

ip_server::ip_server(asio::io_context &ioContext)
    : ioContext(ioContext),
      sslContext(asio::ssl::context::tlsv13_server),
      acceptor(ioContext) { }

std::expected<ip_server, std::string> ip_server::create(asio::io_context& ioContext, const std::optional<keyfiles>& keys) {
    ip_server server(ioContext);

    try {
        if (keys) {
            server.use_tls = true;

            server.sslContext.use_certificate_chain_file(keys->certFile);

            auto format = determineFileFormat(keys->keyFile);
            server.sslContext.use_private_key_file(keys->keyFile, format);

            server.sslContext.load_verify_file(keys->caFile);
            server.sslContext.set_verify_mode(
                      asio::ssl::verify_peer
                    | asio::ssl::verify_fail_if_no_peer_cert

                    // In a controlled network where IP addresses are static, the client may only need to verify its certificate once.
                    // For now, we will not do this.
                    //| asio::ssl::verify_client_once
            );
        }
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }

    return server;
}

ip_server::~ip_server() = default;

std::expected<void, std::string> ip_server::bind(uint32_t port, std::string_view interface) {
    try {
        acceptor.open(asio::ip::tcp::v4());
        acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));

        if (!interface.empty() && interface != "0.0.0.0" && interface != "::" && interface != "*")
            acceptor.bind(asio::ip::tcp::endpoint(asio::ip::make_address(interface), port));
        else
            acceptor.bind(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));

        acceptor.listen(asio::socket_base::max_listen_connections);
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }

    return {};
}

std::expected<std::unique_ptr<abstract_stream>, std::string> ip_server::awaitConnection() {
    try {
        auto socket = acceptor.accept();

        if (use_tls) {
            asio::ssl::stream<tcp::socket> ssl_stream(std::move(socket), sslContext);
            ssl_stream.handshake(asio::ssl::stream_base::client);
            return std::make_unique<tls_stream>(std::move(ssl_stream));
        } else
            return std::make_unique<plain_stream>(std::move(socket));

    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}


