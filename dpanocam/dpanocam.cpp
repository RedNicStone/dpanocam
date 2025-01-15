
#include "config.h"
#include "ip-server.h"
#include "serial_master.h"

#include <boost/endian.hpp>

#include <iostream>
#include <deque>
#include <iomanip>


template <typename T>
T handle_error(std::expected<T, std::string>&& result) {
    if (!result) {
        std::cerr << result.error() << std::endl;
        exit(1);
    }

    return std::move(result.value());
}

template <>
void handle_error(std::expected<void, std::string>&& result) {
    if (!result) {
        std::cerr << result.error() << std::endl;
        exit(1);
    }
}

uint64_t get_transmission_time(uint64_t bytes, uint64_t bitrate) {
    uint64_t timeInNanoseconds = (bytes * 1000 * 1000) / (bitrate / 8);
    return timeInNanoseconds;
}

void handle_tcp_to_serial(serial_master& serial, ip_server& tcp, uint64_t timeout_ns, bool verbose) {
    std::array<uint8_t, 1024> send_buffer{};
    std::array<uint8_t, 1024> receive_buffer{};
    std::vector<uint8_t> send_message;
    std::vector<uint8_t> receive_message;

    auto tcp_stream = handle_error(tcp.awaitConnection());
    {
        if (verbose) std::cout << "TCP Connection established, awaiting data" << std::endl;

        for (;;) {
            boost::system::error_code ec;
            uint32_t bytes_read = tcp_stream->read_some(boost::asio::buffer(send_buffer), ec);
            send_message.insert(send_message.end(), send_buffer.begin(), send_buffer.begin() + bytes_read);

            if (ec == boost::asio::error::eof)
                break;
            else if (ec) {
                std::cerr << "Error while reading from TCP: " << ec.message() << std::endl;
                return;
            }

            if (verbose) std::cout << "Received " << bytes_read << " bytes from TCP" << std::endl;
        }

        if (verbose) std::cout << "Transaction complete, received a total of " << send_message.size() << " bytes from TCP" << std::endl;
    }

    if (verbose) {
        std::cout << "Sending serial data: ";
        for (uint8_t byte : send_message)
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        std::cout << std::endl;
    }

    {
        if (verbose) std::cout << "Sending data to serial" << std::endl;
        auto serial_stream = serial.get_stream();

        {
            boost::system::error_code ec;
            serial_stream->write(boost::asio::buffer(send_message.data(), send_message.size()), ec);

            if (ec) {
                std::cerr << "Error while writing to serial: " << ec.message() << std::endl;
                return;
            }
        }
        if (verbose) std::cout << "Data sent to serial" << std::endl;

        if (timeout_ns == 0) {
            if (verbose) std::cout << "Skipping serial response as no timeout was specified" << std::endl;
            return;
        }

        boost::asio::steady_timer timer(serial.get_io_context());
        auto timeout = std::chrono::nanoseconds(timeout_ns);
        timer.expires_after(timeout);

        timer.async_wait([&](const boost::system::error_code& ec) {
            if (ec) {
                std::cerr << "Error while waiting for serial: " << ec.message() << std::endl;
                return;
            }
            if (verbose) std::cout << "Serial timeout" << std::endl;
            serial_stream->get_handle().cancel();
        });

        serial_stream->async_read_some(boost::asio::buffer(receive_buffer), [&](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (ec == boost::asio::error::operation_aborted) {
                if (verbose) std::cout << "Serial read cancelled" << std::endl;
                return;
            } else if (!ec) {
                timer.expires_after(timeout);

                receive_message.insert(receive_message.end(), receive_buffer.begin(), receive_buffer.begin() + bytes_transferred);
                if (verbose) std::cout << "Received " << bytes_transferred << " bytes from serial" << std::endl;
            } else {
                std::cerr << "Error while reading from serial: " << ec.message() << std::endl;
                return;
            }
        });

        if (verbose) std::cout << "Received a total of " << receive_message.size() << " bytes from serial" << std::endl;
    }

    {
        if (verbose) std::cout << "Sending data to TCP" << std::endl;
        boost::system::error_code ec;
        tcp_stream->write(boost::asio::buffer(send_message.data(), send_message.size()), ec);

        if (ec) {
            std::cerr << "Error while writing to TCP: " << ec.message() << std::endl;
            return;
        }
        if (verbose) std::cout << "Data sent to TCP" << std::endl;
    }
}


int main(int argc, char** argv) {
    config conf = handle_error(config::parseFromCLI(argc, argv));

    std::cout << "- Starting dpanocam"  << '\n';
    std::cout << "-- OpenSSL version: " << SSLeay_version(SSLEAY_VERSION) << '\n';
    std::cout << "-- Boost version: "   << BOOST_LIB_VERSION << '\n';

    std::cout << "- Initializing ASIO" << '\n';
    boost::asio::io_context io_context;

    std::cout << "- Initializing serial port" << '\n';
    std::cout << "-- Binding to device '" << conf.serial_device << "' with a baud rate of " << conf.serial_baud_rate << '\n';
    auto serial = handle_error(serial_master::create(io_context, conf.serial_device, static_cast<int32_t>(conf.serial_baud_rate)));

    std::cout << "- Initializing network server" << '\n';
    std::cout << "-- Binding to interface " << conf.ip_address << " on port " << conf.ip_port << '\n';
    auto ipServer = handle_error(ip_server::create(io_context));
    handle_error(ipServer.bind(conf.ip_port, conf.ip_address));

    std::cout << "- Ready to accept connections" << '\n';

    auto transmission_time = get_transmission_time(conf.serial_timeout, conf.serial_baud_rate);
    for (;;)
        handle_tcp_to_serial(serial, ipServer, transmission_time, conf.verbose);
}
