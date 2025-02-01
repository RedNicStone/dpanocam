
#include "ip-client.h"

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

ip_client::ip_client(asio::io_context& ioContext)
        : ioContext(ioContext),
          sslContext(asio::ssl::context::tlsv13_client) { }

std::expected<ip_client, std::string> ip_client::create(asio::io_context& ioContext, const std::optional<keyfiles>& keys) {
    ip_client client(ioContext);

    try {
        if (keys) {
            client.use_tls = true;

            client.sslContext.use_certificate_chain_file(keys->certFile);

            auto format = determineFileFormat(keys->keyFile);
            client.sslContext.use_private_key_file(keys->keyFile, format);

            client.sslContext.load_verify_file(keys->caFile);
            client.sslContext.set_verify_mode(
                      asio::ssl::verify_peer
                    | asio::ssl::verify_fail_if_no_peer_cert
            );
        }
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }

    return client;
}

ip_client::~ip_client() = default;

std::expected<void, std::string> ip_client::bind(uint32_t port, std::string_view host) {
    try {

        tcp::resolver resolver(ioContext);
        endpoints = resolver.resolve(host, std::to_string(port));

    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }

    return {};
}

std::expected<std::unique_ptr<abstract_stream>, std::string> ip_client::openConnection() {
    try {

        auto socket = asio::ip::tcp::socket(ioContext);
        auto endpoint = asio::connect(socket, endpoints);

        if (use_tls) {
            asio::ssl::stream<asio::ip::tcp::socket> sslStream(std::move(socket), sslContext);
            sslStream.handshake(asio::ssl::stream_base::client);

            return std::make_unique<tls_stream>(std::move(sslStream));
        } else
            return std::make_unique<plain_stream>(std::move(socket));

    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}

