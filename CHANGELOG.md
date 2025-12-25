# CHANGELOG

Format based on  [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)


## [1.0.2] - 25-12-2025

### Added
- Comprehensive README.md documentation in English covering all project features
- Centralized configuration system with all settings in `include/kroner_config.h`
- Build flag `-Iinclude` in platformio.ini for proper header file resolution
- Fallback DEBUG macros in APCModule for compatibility when config header is not available

### Fixed
- Configuration header renamed from `config.h` to `kroner_config.h` to avoid conflicts with system/library headers
- Include guard updated to `KRONER_CONFIG_H` for unique identification
- APCModule library updated to include `<kroner_config.h>` with angle brackets for proper library path resolution
- Compilation errors in APCModule resolved by adding fallback DEBUG_PRINT/DEBUG_PRINTLN definitions

### Changed
- Moved configuration header from `src/config.h` to `include/kroner_config.h`
- Updated all source files to include `kroner_config.h` instead of `config.h`
- Improved APCModule integration with centralized configuration system
- Build system now explicitly includes `include/` directory for library compilation

## [1.0.1] - 25-12-2025

### Added
- Captive Portal functionality using DNSServer to automatically open browser when connecting to "Kroner" WiFi network
- DNS server configured to redirect all domain requests to 192.168.4.1
- `handleNotFound()` function to redirect unknown routes to root with HTTP 302
- WiFi Access Point "Kroner" with WebServer serving HTML interface from LittleFS
- REST API endpoints for BLE message monitoring: GET `/api/messages`, POST `/api/send`
- HTTP polling system (200ms intervals) for real-time message display
- Web interface with gradient purple UI, message cards, hex display, and timestamps

### Fixed
- INPUT9PIN (GPIO5) configuration with proper pinMode and INPUT_PULLDOWN
- Python notebook serial listener updated to read raw bytes without waiting for newlines
- WebSockets library conflict resolved by implementing HTTP polling alternative
- LittleFS filesystem upload process documented and working
- U8g2 compilation issues fixed by changing lib_ldf_mode to deep

### Changed
- Removed WebSocketsServer dependency in favor of HTTP polling
- Updated `lib_ldf_mode` from `chain+` to `deep` in platformio.ini
- Modified serial reading in Python notebooks to use `ser.read(ser.in_waiting)` for faster detection
- BLE messages now stored in buffer and exposed via REST API

## [1.0.0] - 31-10-2025

### Added
- New `sendInitialSwitchState()` function to send initial switch states when connecting via BLE
- Debug messages to display the initial state of each switch on connection

### Fixed
- Fixed bug where only Switch 2 state was sent on first BLE connection
- Now correctly sends the state of all 3 switches (INPUT7, INPUT8, INPUT9) when establishing BLE connection, regardless of whether they are ON or OFF

### Changed
- Replaced `scanSwitch()` calls with `sendInitialSwitchState()` during BLE connection initialization to ensure all switch states are sent
