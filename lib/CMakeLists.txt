

# Determine if we're building in Debug mode
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(EXTRA_CONFIGURE_FLAGS "--enable-debug")
else()
    set(EXTRA_CONFIGURE_FLAGS "")
endif()

set(LIBMODBUS_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/libmodbus-install)

set(MODBUS ${LIBMODBUS_INSTALL_DIR}/lib/libmodbus.a)
set(MODBUS_INCLUDE_DIR ${LIBMODBUS_INSTALL_DIR}/include)

file(MAKE_DIRECTORY "${MODBUS_INCLUDE_DIR}")

include(ExternalProject)
ExternalProject_Add(
        libmodbus_install
        GIT_REPOSITORY https://github.com/stephane/libmodbus.git
        GIT_TAG v3.1.11
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libmodbus
        CONFIGURE_COMMAND ./autogen.sh && ./configure --prefix=${LIBMODBUS_INSTALL_DIR} --enable-static ${EXTRA_CONFIGURE_FLAGS}
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        BUILD_IN_SOURCE 1
        UPDATE_DISCONNECTED true
        BUILD_BYPRODUCTS ${MODBUS}
)

add_library(libmodbus STATIC IMPORTED)
set_target_properties(libmodbus PROPERTIES
        IMPORTED_LOCATION "${MODBUS}"
        INTERFACE_INCLUDE_DIRECTORIES "${MODBUS_INCLUDE_DIR}"
        IMPORTED_GLOBAL TRUE
)

add_dependencies(libmodbus libmodbus_install)

if (${ENABLE_PTZCTRL})
    set(SDL_AUDIO OFF)        # Disable audio support
    set(SDL_RENDER OFF)       # Disable rendering support
    set(SDL_VIDEO OFF)        # Disable video support
    set(SDL_EVENTS ON)        # Enable event handling
    set(SDL_JOYSTICK ON)      # Enable joystick support
    set(SDL_HAPTIC OFF)       # Disable haptic feedback support
    set(SDL_HIDAPI OFF)       # Disable HID API support
    set(SDL_POWER OFF)        # Disable power management
    set(SDL_THREADS OFF)      # Disable threading support
    set(SDL_TIMERS OFF)       # Disable timer support
    set(SDL_FILE OFF)         # Disable file I/O
    set(SDL_SENSOR OFF)       # Disable sensor support
    set(SDL_LOCALE OFF)       # Disable locale support
    set(SDL_DIALOG OFF)       # Disable dialog support

    set(SDL_OPENGL OFF)       # Disable OpenGL support

    set(SDL_STATIC ON)        # Enable static linking
    set(SDL_SHARED OFF)       # Disable shared linking

    set(SDL_UNIX_CONSOLE_BUILD ON)  # Disable windowing support

    include(FetchContent)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG main
    )

    FetchContent_MakeAvailable(SDL3)
endif()

