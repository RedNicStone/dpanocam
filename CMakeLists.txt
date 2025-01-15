cmake_minimum_required(VERSION 3.20)
project(dpanocam)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(lib)
#find_library(modbus REQUIRED NAMES modbus)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system program_options)
find_package(OpenSSL REQUIRED)

set(ENABLE_PTZCTRL OFF CACHE BOOL "Enable input")
set(SDL_TARGET)
set(EXTRA_DEFINES)
if (${ENABLE_PTZCTRL})
    file(GLOB_RECURSE PTZCTRL_SOURCES ptzctrl/*.cpp)

    add_executable(ptzctrl ${PTZCTRL_SOURCES})
    target_link_libraries(ptzctrl
            PRIVATE
            Boost::system
            Boost::program_options
            OpenSSL::SSL OpenSSL::Crypto
            SDL3-static
    )
endif()


file(GLOB_RECURSE RELAYCTRL_SOURCES relayctrl/*.cpp)

add_executable(relayctrl ${RELAYCTRL_SOURCES})
target_link_libraries(relayctrl
        PRIVATE
        libmodbus
        Boost::program_options
)
add_dependencies(relayctrl libmodbus_install)


file(GLOB_RECURSE DPANOCAM_SOURCES dpanocam/*.cpp)

add_executable(dpanocam ${DPANOCAM_SOURCES})
target_link_libraries(dpanocam
    PRIVATE
        Boost::system
        Boost::program_options
        OpenSSL::SSL OpenSSL::Crypto
)
