
#include "relay-axcx4020.h"


std::expected<RelayAXCX4020, std::string> RelayAXCX4020::create(std::string_view serialPort, int baudRate, int slaveId, bool verbose) {
    // The AXCX4020 relay board is a Modbus RTU slave.
    // By default, it uses no parity, 1 stop bit and 8 data bits.
    // Also by default, the baud rate is 9600 and the slave ID is 1.
    // These are expected to change so they are made parameters.

    RelayAXCX4020 relay;
    relay.slaveId = slaveId;
    // Create a new Modbus RTU context for the device.
    relay.ctx = modbus_new_rtu(serialPort.data(), baudRate, 'N', 8, 1);

    if (!relay.ctx)
        return std::unexpected("Failed to create Modbus RTU context");

    modbus_set_debug(relay.ctx, verbose);

    int32_t rc;
    /*
    // Setting the serial mode is not typically required. If it is, we also need permissions to modify the serial port.
    rc = modbus_rtu_set_serial_mode(relay.ctx, MODBUS_RTU_RS485);
    if (rc < 0) {
        auto errorString = modbus_strerror(errno);
        modbus_free(relay.ctx);
        relay.ctx = nullptr;

        return std::unexpected("Failed to set serial mode: " + std::string(errorString));
    }
    */

    modbus_set_byte_timeout(relay.ctx, 5, 0);
    modbus_set_response_timeout(relay.ctx, 5, 0);
    modbus_set_indication_timeout(relay.ctx, 5, 0);

    rc = modbus_set_slave(relay.ctx, slaveId);
    if (rc < 0)
        return std::unexpected("Failed to set slave");

    rc = modbus_connect(relay.ctx);
    if (rc < 0) {
        auto errorString = modbus_strerror(errno);
        modbus_free(relay.ctx);
        relay.ctx = nullptr;

        return std::unexpected("Failed to create Modbus RTU context: " + std::string(errorString));
    }

    // Attempt to enable error recovery.
    // This is not required, but it will make the communication more robust.
    /*
    modbus_set_error_recovery(
            relay.ctx,
            static_cast<modbus_error_recovery_mode>(MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL)
    );
     */

    modbus_flush(relay.ctx);
    return relay;
}


std::expected<void, std::string> RelayAXCX4020::setRelayState(RelayIndex relayIndex, RelayState state) {
    if (!ctx) [[unlikely]] return std::unexpected("Modbus context is not initialized");

    auto rc = modbus_write_bit(ctx, relayIndex, state);
    if (rc < 0) {
        auto errorString = modbus_strerror(errno);
        return std::unexpected("Failed to write relay state: " + std::string(errorString));
    }

    return {};
}


RelayAXCX4020::~RelayAXCX4020() {
    if (!ctx) return;

    modbus_close(ctx);
    modbus_free(ctx);
}
