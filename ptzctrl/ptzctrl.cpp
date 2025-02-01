
#include "pelco-d.h"
#include "config.h"

#include <SDL3/SDL.h>
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

pelco_d::MovementCommand snapToDirection(float x, float y) {
    // Deadzone
    constexpr float theta = 0.15;
    if (std::max(x, -x) < theta && std::max(y, -y) < theta)
        return pelco_d::MovementCommand::Stop;

    float magnitude = std::sqrt(x * x + y * y);
    float normX = x / magnitude;
    float normY = y / magnitude;

    std::tuple<float, float, uint16_t> directions[] = {
            { -1.0f, +1.0f, pelco_d::MovementCommand::PanRight | pelco_d::MovementCommand::TiltUp   },
            { -1.0f, -1.0f, pelco_d::MovementCommand::PanRight | pelco_d::MovementCommand::TiltDown },
            { +1.0f, +1.0f, pelco_d::MovementCommand::PanLeft  | pelco_d::MovementCommand::TiltUp   },
            { +1.0f, -1.0f, pelco_d::MovementCommand::PanLeft  | pelco_d::MovementCommand::TiltDown },
    };
    std::tuple<float, float, uint16_t> closestDirection = { 0.0f, 0.0f, pelco_d::MovementCommand::Stop };
    float maxDotProduct = -1.0f;

    for (const auto& dir : directions) {
        float dotProduct = normX * std::get<0>(dir) + normY * std::get<1>(dir);
        if (dotProduct > maxDotProduct) {
            maxDotProduct = dotProduct;
            closestDirection = dir;
        }
    }

    return static_cast<pelco_d::MovementCommand>(std::get<2>(closestDirection));
}

int main(int argc, char** argv) {
    config conf = handle_error(config::parseFromCLI(argc, argv));

    if (conf.verbose) {
        std::cout << "- Starting dpanocam"  << '\n';
        std::cout << "-- OpenSSL version: " << SSLeay_version(SSLEAY_VERSION) << '\n';
        std::cout << "-- Boost version: "   << BOOST_LIB_VERSION << '\n';
        std::cout << "-- SDL version: "  << SDL_GetRevision() << '\n';

        std::cout << "- Initializing ASIO" << '\n';
    }
    boost::asio::io_context io_context;

    if (conf.verbose) {
        std::cout << "- Initializing network client" << '\n';
        std::cout << "-- Binding to interface " << conf.ip_address << " on port " << conf.ip_port << '\n';
    }
    auto ipClient = handle_error(ip_client::create(io_context));
    handle_error(ipClient.bind(conf.ip_port, conf.ip_address));

    if (conf.verbose)
        std::cout << "-- Connecting to server" << '\n';

    pelco_d pelco(ipClient);
    pelco.setDevice(conf.device_id);
    pelco.setVerbose(conf.verbose);

    switch (conf.mode) {
        case config::mode::goto_point: {
            if (conf.verbose)
                std::cout << "- Going to preset " << static_cast<uint32_t>(conf.goto_location) << '\n';
            handle_error(pelco.sendCustomCommand(0x0007, conf.goto_location));
            return 0;
        }
        case config::mode::query: {
            if (conf.verbose)
                std::cout << "- Querying current location" << '\n';
            auto pan = handle_error(pelco.sendCustomCommand(0x0051, 0x0000));
            auto tilt = handle_error(pelco.sendCustomCommand(0x0053, 0x0000));

            if (pan.size() != 7 || tilt.size() != 7) {
                std::cerr << "Invalid response from device" << std::endl;
                return 1;
            }

            if (pan[0] != 0xFF || tilt[0] != 0xFF ||
                pan[1] != conf.device_id || tilt[1] != conf.device_id ||
                pan[2] != 0x00 || tilt[2] != 0x00 ||
                pan[3] != 0x59 || tilt[3] != 0x5B) {
                std::cerr << "Invalid response from device" << std::endl;
                return 1;
            }

            const uint16_t pan_pos  = *reinterpret_cast<uint16_t*>(&pan[4]);
            const uint16_t tilt_pos = *reinterpret_cast<uint16_t*>(&tilt[4]);

            std::cout << static_cast<uint32_t>(pan_pos) << ',' << static_cast<uint32_t>(tilt_pos) << std::endl;

            return 0;
        }
        case config::mode::interactive:
            break;
    }

    if (conf.verbose)
        std::cout << "- Connecting to gamepad" << '\n';

    if (!SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_EVENTS)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!SDL_HasGamepad()) {
        std::cerr << "No gamepads connected" << std::endl;
        SDL_Quit();
        return 0;
    }

    int32_t numGamepads;
    auto* gamepads = SDL_GetGamepads(&numGamepads);
    if (conf.verbose)
        std::cout << "Number of gamepads: " << numGamepads << std::endl;

    const uint32_t gamepad_id = gamepads[0];
    SDL_free(gamepads);

    SDL_Gamepad* gamepad = SDL_OpenGamepad(gamepad_id);
    if (gamepad == nullptr) {
        std::cerr << "Failed to open gamepad: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    const char* gamepadName = SDL_GetGamepadName(gamepad);
    std::cout << "Gamepad connected: " << (gamepadName ? gamepadName : "Unknown") << std::endl;

    SDL_SetGamepadEventsEnabled(true);

    bool saveSlotConfirmed = false;
    bool quit = false;
    SDL_Event event;

    uint8_t saveSlot = 0;
    pelco_d::MovementCommand last_command = pelco_d::MovementCommand::Stop;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;
                case SDL_EVENT_GAMEPAD_REMOVED:
                    if (event.cdevice.which == SDL_GetGamepadID(gamepad)) {
                        std::cout << "Gamepad disconnected" << std::endl;
                        quit = true;
                    }
                    break;
                case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                    if (event.cdevice.which != SDL_GetGamepadID(gamepad))
                        break;
                    switch (event.gbutton.button) {
                        case SDL_GAMEPAD_BUTTON_EAST:
                            handle_error(pelco.sendCommand(pelco_d::MovementCommand::Stop, pelco_d::MovementSpeed::Fast, pelco_d::MovementSpeed::Fast));
                            conf.device_id = conf.device_id == 1 ? 2 : 1;
                            std::cout << "Switching to device " << static_cast<uint32_t>(conf.device_id) << '\n';
                            SDL_SetGamepadLED(gamepad, conf.device_id == 1 ? 255 : 0, 0, conf.device_id == 2 ? 255 : 0);
                            pelco.setDevice(conf.device_id);

                            if (saveSlotConfirmed)
                                std::cout << "Saving to preset aborted\n";
                            saveSlotConfirmed = false;
                            break;
                        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
                            std::cout << "Calling preset from slot " << static_cast<uint32_t>(saveSlot) << '\n';
                            handle_error(pelco.sendCustomCommand(0x0007, saveSlot));
                            saveSlotConfirmed = false;
                            break;
                        case SDL_GAMEPAD_BUTTON_DPAD_UP:
                            if (!saveSlotConfirmed) {
                                SDL_RumbleGamepad(gamepad, 0x8FFF, 0x8FFF, 100);
                                std::cout << "You are trying to save a preset to slot " << static_cast<uint32_t>(saveSlot) << ". To confirm press DPAD UP again\n";
                                saveSlotConfirmed = true;
                            } else {
                                SDL_RumbleGamepad(gamepad, 0x0, 0x8FFF, 50);
                                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                SDL_RumbleGamepad(gamepad, 0x0, 0x8FFF, 50);
                                std::cout << "Saving preset to slot " << static_cast<uint32_t>(saveSlot) << '\n';
                                handle_error(pelco.sendCustomCommand(0x0003, saveSlot));
                                saveSlotConfirmed = false;
                            }
                            break;
                        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
                            saveSlot--;
                            goto print_slot;
                        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
                            saveSlot++;
                            print_slot:
                            std::cout << "Switched to slot " << static_cast<uint32_t>(saveSlot) << '\n';
                            SDL_SetGamepadPlayerIndex(gamepad, saveSlot);

                            if (saveSlotConfirmed)
                                std::cout << "Saving to preset aborted\n";
                            saveSlotConfirmed = false;
                            [[fallthrough]];
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }

        Sint16 axisX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
        Sint16 axisY = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);

        float normalizedX = static_cast<float>(axisX) / 32767.0f;
        float normalizedY = static_cast<float>(axisY) / 32767.0f;

        auto speedX = static_cast<uint8_t>(std::max(normalizedX, -normalizedX) / 2.2f * 0x3f);
        auto speedY = static_cast<uint8_t>(std::max(normalizedY, -normalizedY) / 2.2f * 0x3f);

        if (conf.lock_x)
            speedX = 0x0;
        if (conf.lock_y)
            speedY = 0x0;

        auto command = snapToDirection(normalizedX, normalizedY);
        if (command == last_command && command == pelco_d::MovementCommand::Stop)
            continue;
        last_command = command;

        handle_error(pelco.sendCommand(
                command,
                static_cast<pelco_d::MovementSpeed>(speedX),
                static_cast<pelco_d::MovementSpeed>(speedY)
        ));

        SDL_Delay(50);
    }

    SDL_CloseGamepad(gamepad);
    SDL_Quit();

    return 0;
}
