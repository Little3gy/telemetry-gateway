# ESP32 WiFi Gateway - AI Agent Instructions

## Project Overview

This is an **ESP32 WiFi relay gateway** that bridges UART communication from an STM32 microcontroller with network connectivity. The device:
- Receives data via UART (Serial2 on GPIO16/17) from an external STM32 microcontroller
- Forwards received data over UDP broadcast on port 4210
- Built for WEMOS D1 R32 (ESP32-based development board)

**Current Status**: Working prototype with functional UART-to-UDP relay, but minimal configuration and security hardening.

## Build & Development

### PlatformIO Setup
- **Build System**: PlatformIO (version 60119)
- **Platform Target**: `espressif32` with Arduino framework
- **Board**: WEMOS D1 R32 (variant: `wemos_d1_uno32`)
- **Source Location**: `src/main.cpp`

### Build Commands
```bash
# Build the project
platformio run

# Build and upload to device
platformio run --target upload

# Calculate program size
platformio run --target size

# Erase ESP32 flash
platformio run --target erase

# Upload filesystem
platformio run --target uploadfs

# Debug launch configurations available in VS Code launch.json
```

### Current Limitations
- No explicit dependencies or build flags in `platformio.ini` (relies on defaults)
- No serial port or upload speed configuration (uses defaults)
- No test framework configured despite test/ directory existing

## Architecture & Code Structure

### Main Components
- **main.cpp**: Single source file containing setup() and loop()
  - WiFi initialization with hardcoded credentials
  - UART Serial2 reader interrupt-driven
  - UDP broadcast sender
- **include/**: Ready for header files (currently empty)
- **lib/**: Ready for local libraries (currently empty)

### Hardware Pinout (WEMOS D1 R32)
- **UART Serial2**: GPIO16 (RX) / GPIO17 (TX) - Connected to external STM32
- **WiFi**: Built-in ESP32 WiFi module
- **UDP Broadcast**: Port 4210

### Communication Flow
1. External STM32 sends data via UART to ESP32 (Serial2)
2. ESP32 reads data in interrupt-driven buffer
3. ESP32 transmits buffer contents via UDP broadcast
4. Cycle repeats

## Development Conventions

### Code Style
- Arduino-style `setup()`/`loop()` pattern
- Procedural code without complex abstractions
- Serial communication via string-based protocol

### Project Layout
- Follows strict PlatformIO standard structure
- Git ignores: `.pio/` build artifacts, `.vscode/` generated files
- Auto-generated: `c_cpp_properties.json`, `launch.json`

## Critical Issues & Pitfalls

### 🔴 Security Concerns
1. **Hardcoded WiFi Credentials**: SSID and password in plaintext source code
   - **Fix**: Implement NVS (Non-Volatile Storage) or secure credential management
   - Agents should flag any PR that adds or maintains hardcoded credentials

2. **UDP Broadcast Exposure**: No encryption or authentication on network communication
   - Consider adding HMAC verification or encryption for sensitive data

### ⚠️ Functional Limitations
1. **Blocking Serial Read**: `Serial2.read()` in main loop could starve event loop
   - Monitor for WiFi disconnections or missed packets
   - Consider using FreeRTOS task instead of loop-based reading

2. **No WiFi Error Handling**: Setup connects once but doesn't validate or reconnect if connection drops
   - Add WiFi status monitoring and reconnection logic

3. **No UDP Acknowledgment**: Assumes successful packet delivery
   - Data loss possible; no retry or acknowledgment mechanism

### 📋 Missing Infrastructure
- No unit tests (test/ directory is empty)
- No configuration abstraction (board variant, serial speed, credentials)
- No error logging or debugging output
- Minimal inline documentation

## When Working on This Project

### Before Building
- Verify `platformio.ini` has correct board target and framework
- Ensure PlatformIO extension is installed in VS Code
- Check serial port connection to ESP32

### Code Changes
- Always flag hardcoded credentials or configuration values
- Add error handling for WiFi operations
- Consider blocking operations that could stall the event loop
- If modifying UART handling, verify buffer sizes and overflow protection

### Testing
- Manual testing: Monitor UDP packets with netcat: `nc -l -u 4210`
- Verify data integrity from STM32 through gateway to network
- Test WiFi reconnection scenarios

### Configuration
- Serial2 speed: Currently 115200 baud (hardcoded, used for both USB and STM32 UART)
- WiFi broadcast port: 4210
- Buffer size: Check main.cpp for serial buffer limits

## Related Projects
- **telemetry_gateway**: Another gateway project in the workspace (see `/home/mumu/STM32CubeIDE/workspace_2.1.1/telemetry_gateway`)
- Both likely part of the same multi-device monitoring system

## Recommended Next Steps
1. **Immediate**: Move WiFi credentials to NVS or configuration file
2. **Short-term**: Add WiFi reconnection logic and UDP error handling
3. **Medium-term**: Implement unit tests using PlatformIO's Unity framework
4. **Long-term**: Refactor blocking serial read into FreeRTOS task for better stability
