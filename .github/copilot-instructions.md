# Push Push Air - AI Coding Agent Instructions

## Project Overview
Push Push Air is an ESP32-based battery-powered Bluetooth keyboard pedalboard. It sends configurable keystrokes (e.g., page turning for PDF readers) and is configured via WiFi access point.

## Architecture

### Hardware Abstraction Pattern
Hardware pin assignments use a **conditional compilation pattern** via [ESP32/PushPushAirESP32/customize.h](../ESP32/PushPushAirESP32/customize.h):
- Main code includes `#include "customize.h"` at top
- All hardware pins (`PEDAL1_PIN`, `PEDAL2_PIN`, LED pins) have defaults in [PushPushAirESP32.ino](../ESP32/PushPushAirESP32/PushPushAirESP32.ino) using `#ifndef` guards
- Users override by creating `customize.h` with their pin assignments
- **Critical**: Always use `#ifndef`/`#endif` guards when adding new hardware-related defines

### Dual-Mode Operation
The device has two distinct operational modes triggered at boot:

1. **Normal Mode**: Bluetooth keyboard operation
   - Persistent status LED blinks (frequency indicates battery level)
   - Pedal presses send configured keystrokes via BLE
   
2. **Configuration Mode**: WiFi Access Point (triggered by holding right pedal at boot)
   - Status LED steady blue
   - AP SSID/password from preferences (default: "PushPushAIR2" / "12345678")
   - Web server at 192.168.4.1 serves HTML forms for configuration
   - Auto-shutdown after 5 minutes idle (`CONFIG_MAX_IDLE_TIME`)

3. **Factory Reset Mode**: Both pedals held for 5 seconds at boot resets password

### State Management
Uses ESP32 `Preferences` library for non-volatile storage:
- Namespace: `"pushpush-config"`
- Stored values: `ssid`, `password`, `pedal1` (key index), `pedal2` (key index)
- **Pattern**: Always provide defaults in `getInt()`/`getString()` calls (e.g., `PEDAL1_DEFAULT_KEY_INDEX`)

### Key Selection System
Available keys defined in `key_options[]` struct array ([L122-135](../ESP32/PushPushAirESP32/PushPushAirESP32.ino#L122-L135)):
- Each entry has `.label` (String) and `.value` (uint8_t from BleKeyboard constants)
- Preferences store **index** into this array, not the key value itself
- Web UI generates HTML `<option>` list from this array in `optionsList()` function

## Build Configuration

### Arduino IDE Settings
- **Board**: "WeMos WiFi&Bluetooth Battery" (or any ESP32 dev board)
- **Partition Scheme**: "Huge APP (3MB No OTA/1MB SPIFFS)" - **critical** for fitting BLE + WiFi
- No Makefile or build scripts; uses Arduino IDE compilation

### Required Libraries
- `18650CL` (Pangodream_18650_CL) - battery monitoring
- `BleKeyboard` v0.3.2beta - Bluetooth keyboard emulation
- `Preferences` (built-in ESP32)
- `Bounce2` - button debouncing
- `WiFi`, `AsyncTCP`, `ESPAsyncWebServer` - WiFi configuration mode

## Battery vs Non-Battery Builds
Controlled by `BATTERY_POWERED` define:
- When `true`: includes `Pangodream_18650_CL`, reports battery level to BLE, LED blink rate varies with charge
- When `false`: omits battery code, LED blinks at constant rate
- **Pattern**: Wrap battery-specific code in `#if BATTERY_POWERED` preprocessor blocks

## Web Configuration Implementation
HTML embedded as `PROGMEM` strings ([L149-167](../ESP32/PushPushAirESP32/PushPushAirESP32.ino#L149-L167)):
- Uses placeholders like `%HEAD_PLACEHODER%`, `%SELECT_PLACEHODER%`
- Replaced by `processor()` function before sending to client
- Routes: `/` (config form), `/save` (save preferences), `/end` (shutdown AP)
- Query parameters parsed with `request->getParam()` and stored to Preferences

## Common Workflows

### Adding a New Keystroke Option
1. Add entry to `key_options[]` array with label and BleKeyboard constant
2. Update `key_options_num` (auto-calculated from sizeof)
3. No other changes needed - web UI auto-generates from array

### Adding a New Hardware Pin
1. Define in [customize.h](../ESP32/PushPushAirESP32/customize.h) (user file)
2. Add `#ifndef` fallback in [PushPushAirESP32.ino](../ESP32/PushPushAirESP32/PushPushAirESP32.ino) main file
3. Document in README hardware customization section

### Debugging
- Serial output at 115200 baud shows: boot diagnostics, pedal events, battery levels, web server activity
- Status LED behaviors indicate mode (blink = normal, steady = config, fast blink = factory reset waiting)

## Project-Specific Conventions
- LED behavior inverted via `LED_ON`/`LED_OFF` defines (accommodates different board polarities)
- Pedal state uses inverted logic: `INPUT_PULLUP` means `LOW` when pressed, checked as `== 0`
- Time tracking uses `unsigned long` + `millis()` for non-blocking delays
- All user-facing strings (web UI) embedded in C++ code, not separate files
