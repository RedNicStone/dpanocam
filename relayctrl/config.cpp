
#include "config.h"

#include <boost/program_options.hpp>

#include <sstream>


namespace {
    constexpr bool             default_verbose          = false;
    constexpr std::string_view default_serial_device    = "/dev/ttyUSB0";
    constexpr uint32_t         default_serial_baud_rate = 9600;
}

std::expected<config, std::string> config::parseFromCLI(int argc, char** argv) {
    namespace po = boost::program_options;
    config config;

    po::options_description desc("relayctrl (Modbus relay control)\nAllowed options:");
    desc.add_options()
            ("help,h",              "Display help message")
            ("relay-number",        po::value<int32_t>(),                                                           "Relay number")
            ("relay-state",         po::value<bool>(),                                                              "Relay state (0 = off, 1 = on)")
            ("modbus-address,a",    po::value<uint16_t>()->default_value(0),                                        "Modbus address, default is 0 (broadcast)")
            ("verbose,v",           po::bool_switch(&config.verbose),                                               "Enable verbose output")
            ("serial-device,d",     po::value<std::string>()->default_value(std::string(default_serial_device)),    "Serial device")
            ("serial-baud-rate,b",  po::value<uint32_t>()->default_value(default_serial_baud_rate),                 "Serial baud rate");

    po::positional_options_description positionalOptions;
    positionalOptions.add("relay-number", 1);
    positionalOptions.add("relay-state", 1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions).run(), vm);
        po::notify(vm);

        if (vm.count("help") || vm.empty()) {
            std::stringstream ss;
            ss << desc;
            return std::unexpected(ss.str());
        }

        config.serial_device            = vm["serial-device"].as<std::string>();
        config.serial_baud_rate         = vm["serial-baud-rate"].as<uint32_t>();
        config.modbus_address           = vm["modbus-address"].as<uint16_t>();

        if (!vm.count("relay-number")) {
            std::stringstream ss;
            ss << "Error: relay-number is required" << std::endl;
            ss << desc;
            return std::unexpected(ss.str());
        }

        if (!vm.count("relay-state")) {
            std::stringstream ss;
            ss << "Error: relay-state is required" << std::endl;
            ss << desc;
            return std::unexpected(ss.str());
        }

        config.relay_number             = vm["relay-number"].as<int32_t>();
        config.relay_state              = vm["relay-state"].as<bool>();

    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error: " << e.what() << "\n";
        ss << desc << "\n";
        return std::unexpected(ss.str());
    }

    return config;
}

