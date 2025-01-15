
Tools made available by the dpanocam project:
- dpanocam: A daemon for controlling any serial device, including panorama cameras.
- ptzctrl: A remote control for PTZ (Pan-Tilt-Zoom) cameras that talks to dpanocam.
- relayctrl: A utility to control relays over Modbus.

Below is a quick usage guide for each tool. \
Note that each of these programs will fail with error code 1 if any error occurs and print a message to stderr.

---

## ptzctrl - PTZ Camera Controller
ptzctrl lets you remotely control your PTZ camera. You can send it to a preset location or take manual control using a gamepad.

### Usage
`ptzctrl [options]`

### Options
- -h, --help: Show the help message.
- -v, --verbose: Enable verbose output (more detailed logs).
- -i, --interactive: Interactive mode—control the camera with a gamepad.
- -g, --goto <preset>: Move the camera to a saved preset location (0 to 127). Can't be used with --interactive.
- -d, --device-id <id>: Set the camera's device ID (default is 1). 
- -a, --ip-address <address>: IP address to bind to (default is 0.0.0.0).
- -p, --ip-port <port>: Port to bind to (default is 8080).
- -t, --tls-enable: Enable TLS for secure connections.
- -k, --tls-keyfile <path>: Path to the TLS key file.
- -c, --tls-certfile <path>: Path to the TLS certificate file.
- -f, --tls-ca-file <path>: Path to the TLS CA file.

### Examples
#### Interactive Control (see below for gamepad controls):
`ptzctrl --interactive -a 192.168.1.10`

#### Go to Preset 10:
`ptzctrl --goto 10 -a 192.168.1.10`

### Interactive Control with Gamepad

ptzctrl supports interactively controlling the camera with a gamepad. \
The speed of movement is proportional to the position of the gamepad's analog stick. \
Up to two cameras can be controlled at the same time. \
It's possible to save the current position to a slot and recall it later. \
This is persistent and can be used in conjunction with the --goto option to recall at a later time. \
The current slot index will be printed to stdout.

#### Gamepad Mapping

| Button             | Action                                |
|--------------------|---------------------------------------|
| DPAD Up            | Save current position to current slot |
| DPAD Down          | Call preset from current slot         |
| DPAD Left          | Decrement current slot index          |
| DPAD Right         | Increment current slot index          |
| East Symbol Button | Switch to other device                |
| Right Stick        | Move camera in pointed direction      |

---

## dpanocam - Panorama Camera Daemon
dpanocam is a daemon that manages communication with any serial device over TCP/IP. \
This can be used in conjunction with ptzctrl to control panorama cameras. \
Packages are sent as-is over serial and a response is awaited (if --serial-timeout is set). \
If no response was received within the timeout specified, an empty response is sent back.

Encryption is supported via server authenticated TLS. This means that the server will only accept connections from clients that have a valid certificate. \
This may be used in a situation where you want to restrict access to the server to only trusted clients.


### Usage
`dpanocam [options]`

### Options
- -h, --help: Show the help message.
- -v, --verbose: Enable verbose output.
- -d, --serial-device <device>: Serial device to use (default is /dev/ttyUSB0).
- -b, --serial-baud-rate <rate>: Serial baud rate (default is 9600).
- -s, --serial-timeout <seconds>: Timeout in milliseconds for serial communication (default is 8).\
   If > 0, the server waits for a response and resets the timeout on data receipt. \
   If 0, no TCP packet will be sent back.
- -a, --ip-address <address>: IP address to bind to (default is 0.0.0.0).
- -p, --ip-port <port>: Port to bind to (default is 8080).
- -t, --tls-enable: Enable TLS for secure connections.
- -k, --tls-keyfile <path>: Path to the TLS key file.
- -c, --tls-certfile <path>: Path to the TLS certificate file.
- -f, --tls-ca-file <path>: Path to the TLS CA file.

### Examples

#### Custom Serial Settings:
`dpanocam --serial-device /dev/ttyS0 --serial-baud-rate 115200`

#### Encrypted Connection:
`dpanocam --tls-enable --tls-keyfile key.pem --tls-certfile cert.pem --tls-ca-file ca.pem`

---

## relayctrl - Modbus Relay Controller
relayctrl lets you control relays via Modbus commands from the command line.

### Usage
`relayctrl [options] <relay-number> <relay-state>`

`<relay-number>`: The number of the relay you want to control. \
`<relay-state>`: Desired state (0 for off, 1 for on).

### Options
- -h, --help: Show the help message.
- -v, --verbose: Enable verbose output.
- -a, --modbus-address <address>: Modbus address to use (default is 0, which broadcasts to all devices).
- -d, --serial-device <device>: Serial device to use (default is /dev/ttyUSB0).
- -b, --serial-baud-rate <rate>: Serial baud rate (default is 9600).

### Examples
#### Turn On Relay 2:
`relayctrl 2 1`

#### Turn Off Relay 2 with Specific Modbus Address:
`relayctrl --modbus-address 10 2 0`

# Building
This project uses CMake to build. To build, run the following commands:

### Create a build directory and move into it
`mkdir build && cd build`

### Use CMake to generate the build files
#### Configuring for build with and without SDL3
To build without SDL3 (and therefore without the ptzctrl tool), run the following command: \
`cmake ..`

To build with SDL3 to get all tools, run the following command: \
`cmake .. -DENABLE_PTZCTRL=ON`

Note that building SDL3 will take some time and disk space.

#### Configuring for Debug Mode
To build in debug mode, run the following command: \
`cmake .. -DCMAKE_BUILD_TYPE=Debug`

#### Building with ninja
If you have ninja installed, you can use it to build the project faster. To do this, run the following command: \
`cmake -G Ninja ..`

### Build the project
Once CMake has generated the build files, run the following command to build the project: \
`cmake --build .`

### Running the project
Once the project has been built, you can run the tools by executing the executables in the build directory.
