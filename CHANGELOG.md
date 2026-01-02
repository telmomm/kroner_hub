# CHANGELOG

Format based on  [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)


## [1.0.4] - 02-01-2026

### Added
- WebSocket server implementation on port 81 for real-time message delivery
- Links2004/WebSockets library (v2.4.1) for bidirectional communication
- `broadcastBLEMessage()` function to notify all connected WebSocket clients
- WebSocket event handler `onWebSocketEvent()` for managing client connections
- Client-side WebSocket implementation in index.html with automatic reconnection
- Python-based WebSocket simulator (`websocket_simuñator.ipynb`) with multiple scenarios:
  - Chronometer with variable points accumulation
  - Text messages display
  - Countdown simulation (10 to 0)
  - Score updates in real-time
  - Mixed scenario combining all features

### Changed
- **BREAKING**: Replaced HTTP polling (200ms intervals) with push-based WebSocket architecture
- Data transmission now event-driven: messages sent only when new BLE data arrives
- Removed unnecessary periodic message broadcasts
- Enhanced `taskProcessRadio()` to call `broadcastBLEMessage()` on new BLE data
- Integrated `webSocket.loop()` into `taskHandleWebServer()` for event processing
- Client reconnection logic with 5 retry attempts and 3-second intervals

### Fixed
- Bandwidth optimization: eliminated redundant polling of unchanged data
- Real-time responsiveness improved from 200ms to millisecond-level latency
- Server no longer broadcasts identical messages on every request

### Technical Details
- WebSocket listens on `ws://192.168.4.1:81/`
- Broadcast triggered only when `bleMessageReady` flag is true
- Automatic client-side reconnection with exponential backoff
- Base64 encoding preserved for binary data compatibility
- Message format unchanged: `{"len": int, "time": ms, "data": base64_str}`

## [1.0.3] - 27-12-2025

### Added
- FreeRTOS native task implementation with pinned cores for optimal ESP32 dual-core utilization
- `startSystemTasks()` function to initialize and pin tasks to specific cores
- `printBootBanner()` function in serial_functions for centralized firmware info display
- Task handles and declarations for all system tasks (WebServer, BLE, Inputs, Radio, Debug)
- Volatile qualifiers on shared BLE connection state variables for thread-safe access

### Changed
- **BREAKING**: Complete migration from custom TaskScheduler to FreeRTOS native tasks
- Core 0 assignment: WebServer/DNS (50ms) and Radio (200ms) for WiFi stack compatibility
- Core 1 assignment: BLE (20ms), Inputs (10ms) and Debug (5s) for responsive I/O handling
- Input scanning frequency increased to 100Hz (10ms) for improved keypad responsiveness
- Main loop simplified to idle delay; all work now runs in dedicated FreeRTOS tasks
- Boot banner extraction from setup() to reusable `printBootBanner()` function

### Removed
- Custom TaskScheduler class and all related dependencies
- TaskScheduler.h/cpp files (replaced by FreeRTOS native implementation)
- Cooperative scheduling replaced with preemptive multitasking

### Technical Details
- Task priorities: BLE & Inputs (3), WebServer & Radio (2), Debug (1)
- Stack sizes: WebServer/BLE/Radio (4096), Debug (3072)
- All tasks use `vTaskDelay()` for precise, non-blocking timing
- Architecture now leverages true parallel execution on both ESP32 cores

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
