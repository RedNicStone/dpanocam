
#ifndef DPANOCAM_SERIAL_MASTER_H
#define DPANOCAM_SERIAL_MASTER_H

#include "abstract_ip_stream.h"

#include <boost/asio.hpp>

#include <expected>
#include <string_view>


class serial_master {
    boost::asio::io_context& ioContext;
    boost::asio::serial_port serial;

    explicit serial_master(boost::asio::io_context& context) : ioContext(context), serial(context) {}

public:
    static std::expected<serial_master, std::string> create(boost::asio::io_context& ioContext, std::string_view serialPort, int baudRate = 9600);
    ~serial_master();

    std::unique_ptr<serial_stream> get_stream();
    boost::asio::io_context& get_io_context() { return ioContext; }

    serial_master(const serial_master&) = delete;
    serial_master& operator=(const serial_master&) = delete;

    serial_master(serial_master&& other) noexcept = default;
};


#endif //DPANOCAM_SERIAL_MASTER_H
