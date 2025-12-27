# Kroner Hub

**Version:** 1.0.3  
**Platform:** ESP32 (Adafruit Feather)  
**Framework:** Arduino + PlatformIO + FreeRTOS

## Overview

Kroner Hub is a multi-interface communication hub built on ESP32, designed to bridge multiple communication protocols including WiFi, Bluetooth Low Energy (BLE), and APC220 wireless radio. It features a modular architecture with a web interface, captive portal, and flexible input handling capabilities.

**Architecture:** Built on FreeRTOS native tasks with optimized dual-core utilization for ESP32. Core 0 handles WiFi/networking operations, while Core 1 manages BLE and real-time input processing for maximum responsiveness.

## Key Features

- **FreeRTOS Multi-Core Architecture** with pinned tasks for optimal ESP32 dual-core utilization
  - Core 0: WiFi/WebServer (50ms) + Radio processing (200ms)
  - Core 1: BLE polling (20ms) + Input scanning (10ms) + Debug (5s)
  - True parallel execution with preemptive multitasking
- **WiFi Access Point** with captive portal functionality
- **Web Server** with LittleFS filesystem for HTML/static content
- **BLE (Bluetooth Low Energy)** interface for wireless connectivity
  - Uses both ArduinoBLE and NimBLE libraries
  - Device name: "Kroner-Hub"
  - Custom service for button/input notifications
  - Serial bridge service for data transmission
- **APC220 Radio Module** integration for long-range wireless communication
  - Configurable frequency: 435 MHz
  - UART baudrate: 9600 bps
  - Air baudrate: 9600 bps (RF rate 3)
  - Output power: 9 (13dBm / 20mW)
- **Input System**
  - 3x3 Keypad matrix with debouncing
  - 3 interrupt-driven inputs (F1, F2, F3)
  - 7 additional GPIO inputs
- **Modular Code Structure** for easy maintenance and expansion

## Hardware Requirements

### Main Board
- Adafruit ESP32 Feather or compatible ESP32 board

### Peripherals
- **APC220 Wireless Module**
  - SET pin: GPIO 23
  - RX pin: GPIO 16
  - TX pin: GPIO 17

### Input Configuration
- **Interrupt Inputs:**
  - F1: GPIO 22
  - F2: GPIO 21
  - F3: GPIO 19
  
- **Keypad Matrix:**
  - Rows: GPIO 14, 12, 13
  - Columns: GPIO 32, 33, 27

- **Additional Inputs:**
  - INPUT7: GPIO 15
  - INPUT8: GPIO 4
  - INPUT9: GPIO 5
  - INPUT10: GPIO 18

## Software Architecture

The project uses **FreeRTOS native tasks** with core pinning for optimal performance:

### Task Distribution
- **Core 0 (WiFi Stack):**
  - WebServer Task (50ms, priority 2) - HTTP requests & DNS
  - Radio Task (200ms, priority 2) - APC220 data processing

- **Core 1 (Real-time I/O):**
  - BLE Task (20ms, priority 3) - BLE.poll() & connection handling
  - Inputs Task (10ms, priority 3) - Keypad & switch scanning
  - Debug Task (5s, priority 1) - System status logging

### Module Organization

```
src/
├── main.cpp                    # Setup & FreeRTOS task initialization
├── task_functions.cpp/h        # FreeRTOS task implementations
├── ble_functions.cpp/h         # BLE functionality
├── webserver_functions.cpp/h   # Web server & WiFi AP
├── input_functions.cpp/h       # Input handling & keypad
└── serial_functions.cpp/h      # APC220 + boot banner

include/
└── kroner_config.h             # Centralized configuration

lib/
└── APCModule/                  # Custom APC220 library

data/
└── index.html                  # Web interface (LittleFS)
```

## Configuration

All configuration parameters are centralized in `include/kroner_config.h`:

### WiFi Settings
- **SSID:** `Kroner`
- **Password:** (none - open network)
- **IP Address:** 192.168.4.1
- **Subnet Mask:** 255.255.255.0

### Serial Communication
- **USB Serial:** 115200 baud
- **Radio UART:** 9600 baud
- **Radio Air Rate:** 9600 bps

### Debug Mode
Debug output can be enabled/disabled by setting `DEBUG` to 1 or 0 in `kroner_config.h`.

## Building & Flashing

### Prerequisites
- [PlatformIO](https://platformio.org/) installed
- USB serial drivers for your operating system

### Build Commands

```bash
# Clean and build
platformio run

# Upload firmware
platformio run --target upload

# Upload filesystem (LittleFS)
platformio run --target uploadfs

# Monitor serial output
platformio device monitor
```

### Upload Notes
- Default upload port: `/dev/cu.usbserial-0001` (macOS)
- Monitor speed: 115200 baud
- If upload fails, try holding BOOT button during flash

## Dependencies

The project uses the following libraries (automatically managed by PlatformIO):

- **NimBLE-Arduino** (^1.4.1) - BLE stack
- **ArduinoBLE** (^1.3.6) - Alternative BLE library
- **U8g2** (^2.35.6) - Display library
- **Keypad** (^3.1.1) - Matrix keypad handling
- **EspSoftwareSerial** (^8.2.0) - Software serial
- **APCModule** (custom) - APC220 radio interface

## Web Interface

The web interface is served from LittleFS filesystem and accessible at:
- **Direct:** http://192.168.4.1
- **Captive Portal:** Automatically redirects when connecting to WiFi

### API Endpoints
- `GET /` - Main web interface
- `GET /api/messages` - Retrieve received messages
- `POST /api/send` - Send message via radio
- Captive portal redirection on 404

## BLE Services

### Pulsador Service
- **Service UUID:** `19B10000-E8F2-537E-4F6C-D104768A1214`
- **Characteristics:**
  - Button/Input notifications: `b444ea9a-a1b8-11ee-8c90-0242ac120002`
  - Firmware info: `c555fa9a-a1b8-11ee-8c90-0242ac120003`

### Serial Bridge Service
- **Service UUID:** `12345678-1234-5678-1234-56789abcdef0`
- **Characteristics:**
  - Write data: `12345678-1234-5678-1234-56789abcdef1`

## APC220 Radio Module

The APC220 module provides long-range wireless communication with configurable parameters:

### Default Configuration
```
Frequency: 435000 kHz (435 MHz)
RF Data Rate: 3 (9600 bps)
Output Power: 9 (13dBm / 20mW)
UART Rate: 3 (9600 bps)
Parity: 0 (no check)
```

Configuration string: `PARA 435000 3 9 3 0`

## Development

### Project Structure
- **FreeRTOS Native Tasks:** Preemptive multitasking with core pinning for deterministic behavior
- **Thread-Safe Design:** Volatile qualifiers on shared state variables for multi-core safety
- **Modular Design:** Each subsystem (BLE, WiFi, Inputs, Radio) is isolated in separate files
- **Centralized Configuration:** All pins, settings, and constants in one header file
- **Debug Macros:** Conditional compilation for debug output
- **Interrupt-Driven Inputs:** Efficient handling of time-critical inputs (F1, F2, F3)

### Adding New Features
1. Define new constants in `include/kroner_config.h`
2. Create new module files in `src/` if needed
3. Add initialization call in `main.cpp setup()`
4. Update this README with new features

## Troubleshooting

### Upload Issues
- Ensure correct serial port in `platformio.ini`
- Hold BOOT button while uploading if auto-reset fails
- Check USB cable supports data transfer (not just power)

### WiFi Connection Problems
- Device creates AP named "Kroner" (no password)
- Connect to 192.168.4.1 in browser
- Captive portal should redirect automatically

### BLE Not Visible
- Check BLE is initialized in serial monitor
- Look for device name "Kroner-Hub"
- Restart device if BLE fails to start

### Radio Communication
- Verify APC220 module connections (RX, TX, SET pins)
- Check configuration with `radio.getSettings()`
- Ensure both radios use same frequency and parameters

## Memory Usage

Current build statistics:
- **Flash:** ~79.8% (1,046,345 / 1,310,720 bytes)
- **RAM:** ~16.0% (52,524 / 327,680 bytes)

## License

[Add your license information here]

## Authors

- Telmo M.

## Acknowledgments

- ESP32 Arduino Core team
- NimBLE and ArduinoBLE library authors
- APC220 module community

## Version History

### 1.0.3 (Current)
- **FreeRTOS native tasks** with dual-core pinning
- Replaced custom TaskScheduler with ESP32 native multitasking
- Core 0: WiFi/networking operations
- Core 1: BLE and real-time input processing
- Improved keypad responsiveness (100Hz scan rate)
- Thread-safe shared state management
- Centralized boot banner in `printBootBanner()`
- Task priorities optimized for deterministic I/O handling

### 1.0.2
- Centralized configuration system in `kroner_config.h`
- Build system improvements for library compilation
- APCModule integration fixes

### 1.0.1
- Modular architecture implementation
- WiFi captive portal
- BLE dual-stack support
- APC220 radio integration
- Web interface with REST API
- LittleFS filesystem support

---

For more information or support, please open an issue on the project repository.
