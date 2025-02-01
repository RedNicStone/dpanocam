
#ifndef DPANOCAM_PELCO_D_H
#define DPANOCAM_PELCO_D_H

#include "ip-client.h"

#include <boost/asio.hpp>

#include <expected>
#include <string>


class pelco_d {
    ip_client& client;
    uint8_t slaveId = 0;

public:
    explicit pelco_d(ip_client& client);
    ~pelco_d();

    enum MovementCommand : uint16_t {
        Stop        = 0x00,
        PanLeft     = 0x02,
        PanRight    = 0x04,
        TiltUp      = 0x08,
        TiltDown    = 0x10
    };

    enum MovementSpeed : uint8_t {
        Slow    = 0x00,
        Medium  = 0x20,
        Fast    = 0x3F,
        Turbo   = 0xFF,
    };

    bool verbose = false;

    std::expected<std::vector<uint8_t>, std::string> sendCommand(MovementCommand command, MovementSpeed panSpeed, MovementSpeed tiltSpeed);
    std::expected<std::vector<uint8_t>, std::string> sendCustomCommand(uint16_t command, uint16_t data);

    void setDevice(uint8_t device) { slaveId = device; }
    void setVerbose(bool verbose) { this->verbose = verbose; }

    pelco_d(const pelco_d&) = delete;
    pelco_d& operator=(const pelco_d&) = delete;

    pelco_d(pelco_d&& other) noexcept = default;
};


#endif //DPANOCAM_PELCO_D_H
