
#include "relay-axcx4020.h"
#include "config.h"

#include <iostream>


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

int main(int argc, char *argv[]) {
    config conf = handle_error(config::parseFromCLI(argc, argv));

    if (conf.relay_number < RelayAXCX4020::RelayIndex::Relay1 || conf.relay_number > RelayAXCX4020::RelayIndex::Relay2) {
        std::cerr << "Invalid relay number: " << conf.relay_number << std::endl;
        exit(1);
    }

    auto relay = handle_error(RelayAXCX4020::create(conf.serial_device, static_cast<int32_t>(conf.serial_baud_rate), conf.modbus_address, conf.verbose));

    handle_error(relay.setRelayState(static_cast<RelayAXCX4020::RelayIndex>(conf.relay_number), static_cast<RelayAXCX4020::RelayState>(conf.relay_state)));

    return 0;
}
