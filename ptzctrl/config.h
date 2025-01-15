
#ifndef DPANOCAM_CONFIG_H
#define DPANOCAM_CONFIG_H

#include "ip-client.h"

#include <string>
#include <cstdint>
#include <expected>


struct config {

    bool         verbose;
    bool         interactive;
    bool         lock_x;
    bool         lock_y;
    uint8_t      goto_location;
    uint8_t      device_id;

    std::string  ip_address;
    uint16_t     ip_port;

    bool         tls_enable;
    ip_client::keyfiles tls_keyfiles;

    static std::expected<config, std::string> parseFromCLI(int argc, char** argv);

};


#endif //DPANOCAM_CONFIG_H
