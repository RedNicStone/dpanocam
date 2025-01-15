
#ifndef DPANOCAM_CONFIG_H
#define DPANOCAM_CONFIG_H

#include <string>
#include <cstdint>
#include <expected>


struct config {

    bool         verbose;

    int32_t      relay_number;
    bool         relay_state;

    std::string  serial_device;
    uint32_t     serial_baud_rate;
    uint16_t     modbus_address;

    static std::expected<config, std::string> parseFromCLI(int argc, char** argv);

};


#endif //DPANOCAM_CONFIG_H
