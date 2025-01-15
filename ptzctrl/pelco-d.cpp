
#include "pelco-d.h"

#include <iostream>
#include <iomanip>


using namespace boost::asio;

pelco_d::pelco_d(ip_client& client) : client(client) { }

pelco_d::~pelco_d() = default;

std::expected<void, std::string>
pelco_d::sendCommand(pelco_d::MovementCommand command, pelco_d::MovementSpeed panSpeed, pelco_d::MovementSpeed tiltSpeed) {
    std::array<uint8_t, 7> command_data{};

    // First byte is always 0xFF (sync)
    command_data[0] = 0xFF;

    // Second byte is the slave ID
    command_data[1] = slaveId;

    // Third and fourth byte is the command
    // This is a little-endian byte order, so we need to reverse the order of the bytes
    command_data[2] = static_cast<uint8_t>(command >> 8);
    command_data[3] = static_cast<uint8_t>(command);

    // Fifth and sixth byte is the data (pan/tilt speed)
    command_data[4] = panSpeed;
    command_data[5] = tiltSpeed;

    // Seventh byte is the checksum
    uint8_t checksum = 0;
    for (auto byte = command_data.begin() + 1; byte != command_data.end() - 1; ++byte)
        checksum += *byte;
    command_data[6] = checksum;

    /*
    std::cout << "Sending command: ";
    for (uint8_t byte : command_data)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    std::cout << std::endl;
     */

    // Send the command
    auto stream = client.openConnection();
    if (!stream)
        return std::unexpected(stream.error());
    else {
        boost::system::error_code ec;
        stream->get()->write(boost::asio::buffer(command_data), ec);
        if (ec)
            return std::unexpected(ec.message());
    }

    return {};
}

std::expected<void, std::string> pelco_d::sendCustomCommand(uint16_t command, uint16_t data) {
    std::array<uint8_t, 7> command_data{};

    // First byte is always 0xFF (sync)
    command_data[0] = 0xFF;

    // Second byte is the slave ID
    command_data[1] = slaveId;

    // Third and fourth byte is the command
    // This is a little-endian byte order, so we need to reverse the order of the bytes
    command_data[2] = static_cast<uint8_t>(command >> 8);
    command_data[3] = static_cast<uint8_t>(command);

    // Fifth and sixth byte is the data
    command_data[4] = static_cast<uint8_t>(data >> 8);
    command_data[5] = static_cast<uint8_t>(data);

    // Seventh byte is the checksum
    uint8_t checksum = 0;
    for (auto byte = command_data.begin() + 1; byte != command_data.end() - 1; ++byte)
        checksum += *byte;
    command_data[6] = checksum;

    /*
    std::cout << "Sending command: ";
    for (uint8_t byte : command_data)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    std::cout << std::endl;
     */

    // Send the command
    auto stream = client.openConnection();
    if (!stream)
        return std::unexpected(stream.error());
    else {
        boost::system::error_code ec;
        stream->get()->write(boost::asio::buffer(command_data), ec);
        if (ec)
            return std::unexpected(ec.message());
    }

    return {};
}
