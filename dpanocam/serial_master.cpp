
#include "serial_master.h"

std::expected<serial_master, std::string>
serial_master::create(boost::asio::io_context &ioContext, std::string_view serialPort, int baudRate) {
    boost::asio::io_service ioService;
    boost::asio::serial_port serial(ioService);

    serial_master serial_master(ioContext);
    try {
        serial_master.serial.open(serialPort.data());

        // Pelco D uses no parity, 8 data bits and one stop bit. This is the default for ASIO
        serial_master.serial.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }

    return serial_master;
}

serial_master::~serial_master() = default;

std::unique_ptr<serial_stream> serial_master::get_stream() {
    return std::make_unique<serial_stream>(serial);
}
