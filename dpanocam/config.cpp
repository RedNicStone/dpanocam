
#include "config.h"

#include <boost/program_options.hpp>

#include <sstream>


namespace {
    constexpr bool             default_verbose          = false;
    constexpr std::string_view default_serial_device    = "/dev/ttyUSB0";
    constexpr uint32_t         default_serial_baud_rate = 9600;
    constexpr uint32_t         default_serial_timeout   = 8;
    constexpr bool             default_modbus_enable    = false;
    constexpr std::string_view default_ip_address       = "0.0.0.0";
    constexpr uint16_t         default_ip_port          = 8080;
    constexpr bool             default_tls_enable       = false;
    constexpr std::string_view default_tls_keyfile;
    constexpr std::string_view default_tls_certfile;
    constexpr std::string_view default_tls_ca_file;
}

std::expected<config, std::string> config::parseFromCLI(int argc, char** argv) {
    namespace po = boost::program_options;
    config config;

    po::options_description desc("dpanocam (Panorama camera demon)\nAllowed options:");
    desc.add_options()
            ("help,h",              "Display help message")
            ("verbose,v",           po::bool_switch(&config.verbose),                                               "Enable verbose output")
            ("serial-device,d",     po::value<std::string>()->default_value(std::string(default_serial_device)),    "Serial device")
            ("serial-baud-rate,b",  po::value<uint32_t>()->default_value(default_serial_baud_rate),                 "Serial baud rate")
            ("serial-timeout,s",    po::value<uint32_t>()->default_value(default_serial_timeout),                   "Serial timeout to await response in bytes being sent.\n"
                                                                                                                    "  - If this is greater than zero, the server will wait for a response from the client.\n"
                                                                                                                    "    The server will respond with any data received from the bus. When data is received, this timeout will be reset.\n"
                                                                                                                    "    In case no response is received within the timeout, the server will respond with an empty packet.\n"
                                                                                                                    "  - If this is equal to zero, no tcp packet will be sent back.")
            ("modbus-enable,m",     po::bool_switch(&config.modbus_enable),                                         "Enable Modbus communication")
            ("ip-address,a",        po::value<std::string>()->default_value(std::string(default_ip_address)),       "IP address to bind to")
            ("ip-port,p",           po::value<uint16_t>()->default_value(default_ip_port),                          "Port to bind to")
            ("tls-enable,t",        po::bool_switch(&config.tls_enable),                                            "Enable TLS")
            ("tls-keyfile,k",       po::value<std::string>()->default_value(std::string(default_tls_keyfile)),      "Path to TLS key file")
            ("tls-certfile,c",      po::value<std::string>()->default_value(std::string(default_tls_certfile)),     "Path to TLS certificate file")
            ("tls-ca-file,f",       po::value<std::string>()->default_value(std::string(default_tls_ca_file)),      "Path to TLS CA file");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help") || vm.empty()) {
            std::stringstream ss;
            ss << desc;
            return std::unexpected(ss.str());
        }

        config.serial_device            = vm["serial-device"].as<std::string>();
        config.serial_baud_rate         = vm["serial-baud-rate"].as<uint32_t>();
        config.modbus_enable            = vm["modbus-enable"].as<bool>();
        config.ip_address               = vm["ip-address"].as<std::string>();
        config.ip_port                  = vm["ip-port"].as<uint16_t>();
        config.tls_enable               = vm["tls-enable"].as<bool>();
        config.tls_keyfiles.keyFile     = vm["tls-keyfile"].as<std::string>();
        config.tls_keyfiles.certFile    = vm["tls-certfile"].as<std::string>();
        config.tls_keyfiles.caFile      = vm["tls-ca-file"].as<std::string>();

    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error: " << e.what() << "\n";
        ss << desc << "\n";
        return std::unexpected(ss.str());
    }

    return config;
}

