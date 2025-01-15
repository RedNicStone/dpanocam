
#ifndef DPANOCAM_CONFIG_H
#define DPANOCAM_CONFIG_H

#include "ip-server.h"

#include <string>
#include <cstdint>
#include <expected>


struct config {

    bool         verbose;

    std::string  serial_device;
    uint32_t     serial_baud_rate;
    uint64_t     serial_timeout;
    bool         modbus_enable;

    std::string  ip_address;
    uint16_t     ip_port;

    bool         tls_enable;
    ip_server::keyfiles tls_keyfiles;

    static std::expected<config, std::string> parseFromCLI(int argc, char** argv);

};


#endif //DPANOCAM_CONFIG_H
