
#include "config.h"

#include <boost/program_options.hpp>

#include <sstream>


namespace {
    constexpr int32_t          default_device_id        = 1;
    constexpr std::string_view default_ip_address       = "0.0.0.0";
    constexpr uint16_t         default_ip_port          = 8080;
    constexpr std::string_view default_tls_keyfile;
    constexpr std::string_view default_tls_certfile;
    constexpr std::string_view default_tls_ca_file;
}

std::expected<config, std::string> config::parseFromCLI(int argc, char** argv) {
    namespace po = boost::program_options;
    config config;

    bool interactive = false;
    bool goto_point = false;
    bool query = false;

    po::options_description desc("ptzctrl (PTZ remote control)\nAllowed options:");
    desc.add_options()
            ("help,h",              "Display help message")
            ("verbose,v",           po::bool_switch(&config.verbose),                                               "Enable verbose output")
            ("interactive,i",       po::bool_switch(&interactive),                                                  "Enable interactive mode / input via gamepad")
            ("goto,g",              po::bool_switch(&goto_point),                                                   "Go to a previously saved location, incompatible with --interactive")
            ("query,q",             po::bool_switch(&query),                                                        "Query the current location, incompatible with --interactive")
            ("lock-x",              po::bool_switch(&config.lock_x),                                                "Lock x axis when in interactive mode")
            ("lock-y",              po::bool_switch(&config.lock_y),                                                "Lock y axis when in interactive mode")
            ("device-id,d",         po::value<int32_t>()->default_value(default_device_id),                         "Camera device ID, default is 1. Can be controlled in interactive mode")
            ("ip-address,a",        po::value<std::string>()->default_value(std::string(default_ip_address)),     "IP address to bind to")
            ("ip-port,p",           po::value<uint16_t>()->default_value(default_ip_port),                          "Port to bind to")
            ("tls-enable,t",        po::bool_switch(&config.tls_enable),                                            "Enable TLS")
            ("tls-keyfile,k",       po::value<std::string>()->default_value(std::string(default_tls_keyfile)),    "Path to TLS key file")
            ("tls-certfile,c",      po::value<std::string>()->default_value(std::string(default_tls_certfile)),   "Path to TLS certificate file")
            ("tls-ca-file,f",       po::value<std::string>()->default_value(std::string(default_tls_ca_file)),    "Path to TLS CA file");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.contains("help") || vm.empty()) {
            std::stringstream ss;
            ss << desc;
            return std::unexpected(ss.str());
        }

        if (goto_point) {
            if (interactive) {
                std::stringstream ss;
                ss << "Error: cannot use --goto and --interactive at the same time" << std::endl;
                ss << desc;
                return std::unexpected(ss.str());
            }

            if (query) {
                std::stringstream ss;
                ss << "Error: cannot use --goto and --query at the same time" << std::endl;
                ss << desc;
                return std::unexpected(ss.str());
            }

            if (!vm.contains("device-id")) {
                std::stringstream ss;
                ss << "Must provide --device-id when using --goto" << std::endl;
                ss << desc;
                return std::unexpected(ss.str());
            }

            uint16_t preset = vm["goto"].as<uint16_t>();
            if (preset > 0x7f) {
                std::stringstream ss;
                ss << "Error: preset number must be between 0 and 127" << std::endl;
                ss << desc;
                return std::unexpected(ss.str());
            }

            config.goto_location = static_cast<uint8_t>(preset);
        } else if (interactive) {
            if (query) {
                std::stringstream ss;
                ss << "Error: cannot use --interactive and --query at the same time" << std::endl;
                ss << desc;
                return std::unexpected(ss.str());
            }

            config.mode = config::mode::interactive;
        } else if (query) {
            if (!vm.contains("device-id")) {
                std::stringstream ss;
                ss << "Must provide --device-id when using --query" << std::endl;
                ss << desc;
                return std::unexpected(ss.str());
            }

            config.mode = config::mode::query;
        } else {
            std::stringstream ss;
            ss << "Error: either --goto, --query or --interactive must be specified" << std::endl;
            ss << desc;
            return std::unexpected(ss.str());
        }

        config.device_id                = vm["device-id"].as<int32_t>();
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

