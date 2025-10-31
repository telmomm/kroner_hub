# CHANGELOG

Format based on  [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)


## [1.0.0] - 31-10-2025

### Added
- New `sendInitialSwitchState()` function to send initial switch states when connecting via BLE
- Debug messages to display the initial state of each switch on connection

### Fixed
- Fixed bug where only Switch 2 state was sent on first BLE connection
- Now correctly sends the state of all 3 switches (INPUT7, INPUT8, INPUT9) when establishing BLE connection, regardless of whether they are ON or OFF

### Changed
- Replaced `scanSwitch()` calls with `sendInitialSwitchState()` during BLE connection initialization to ensure all switch states are sent
