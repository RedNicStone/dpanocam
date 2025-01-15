
#ifndef DPANOCAM_RELAY_AXCX4020_H
#define DPANOCAM_RELAY_AXCX4020_H

// This file implements communication with the AXCX4020 relay board.
// Communication is done via the Modbus protocol with a serial hardware link.


#include <modbus/modbus-rtu.h>

#include <string>
#include <string_view>
#include <expected>
#include <utility>


class RelayAXCX4020 {
    modbus_t *ctx;
    int32_t slaveId;

    RelayAXCX4020() { slaveId = 0; ctx = nullptr; }

public:
    enum RelayState : int {
        Off = FALSE,
        On  = TRUE,
    };

    enum RelayIndex : int {
        Relay1 = 0,
        Relay2 = 1,
    };

    static std::expected<RelayAXCX4020, std::string> create(std::string_view serialPort, int baudRate = 9600, int slaveId = 1, bool verbose = false);
    ~RelayAXCX4020();

    std::expected<void, std::string> setRelayState(RelayIndex relayIndex, RelayState state);

    RelayAXCX4020(const RelayAXCX4020&) = delete;
    RelayAXCX4020& operator=(const RelayAXCX4020&) = delete;

    RelayAXCX4020(RelayAXCX4020&& other) noexcept : ctx(std::exchange(other.ctx, nullptr)), slaveId(other.slaveId) {}
    RelayAXCX4020& operator=(RelayAXCX4020&& other)  noexcept {
        ctx = std::exchange(other.ctx, nullptr);
        slaveId = other.slaveId;
        return *this;
    }
};


#endif //DPANOCAM_RELAY_AXCX4020_H
