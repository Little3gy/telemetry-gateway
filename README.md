# Smart Telemetry Gateway
### Fault-Tolerant Wireless Communication System for ROV/Submarine Robotics

A proof-of-concept embedded system that maintains a reliable telemetry link between an ROV and a ground station operator. When the primary Wi-Fi link degrades or fails, the system automatically switches to a LoRa fallback radio — transparently and without operator intervention. All link quality metrics are continuously logged to an SD card for post-mission analysis.

---

## System Architecture

```
┌─────────────────────────────────────────────────────┐
│                    ROV / Vehicle                    │
│                                                     │
│   [Telemetry Source] ──UART──► [STM32F407]          │
│                                    │                │
│                               ┌────┴────┐           │
│                               │  State  │           │
│                               │ Machine │           │
│                               └────┬────┘           │
│                          ┌─────────┴──────────┐     │
│                          │                    │     │
│                      UART│                 SPI│     │
│                          ▼                    ▼     │
│                      [ESP32]            [SX1278]    │
│                      Wi-Fi              LoRa        │
└──────────────────────────────────────────────────── ┘
                          │                    │
                     Wi-Fi│              LoRa  │
                    (UDP) │         (433 MHz)  │
                          ▼                    ▼
┌─────────────────────────────────────────────────────┐
│                  Ground Station                     │
│                                                     │
│          [Python Receiver] ◄──── [CP2102 + LoRa]   │
│          (UDP port 4210)                            │
└─────────────────────────────────────────────────────┘
```

---

## Features

- **Dual-radio architecture** — Primary Wi-Fi (ESP32) with LoRa (SX1278) fallback
- **Real-time link monitoring** — RSSI, packet loss, and latency sampled every 100ms
- **Graceful degradation** — Three-state communication manager: GOOD / DEGRADED / FAILOVER
- **QoS priority queuing** — Critical packets (emergency stop, heading, depth) are always prioritized over non-critical sensor data
- **Black box logging** — Continuous CSV logging to SD card for post-mission analysis
- **FreeRTOS task architecture** — Independent tasks for UART, Wi-Fi, LoRa, link monitoring, and logging

---

## Hardware

| Component | Role | Interface |
|---|---|---|
| STM32F407 Discovery | Main MCU / Brain | — |
| ESP32 WeMos D1 R32 | Primary Wi-Fi radio | UART (USART2) |
| SX1278 Ra-02 (×2) | LoRa fallback radio | SPI1 |
| MicroSD card module | Black box logger | SPI / SDIO |
| CP2102 USB-UART | Laptop LoRa receiver | USB |

---

## Pin Configuration

### STM32F407 → ESP32
| STM32 Pin | ESP32 Pin | Signal |
|---|---|---|
| PA2 (USART2_TX) | GPIO16 (RX2 / D5) | Telemetry data |
| GND | GND | Common ground |

### STM32F407 → SX1278 LoRa
| STM32 Pin | SX1278 Pin | Signal |
|---|---|---|
| PA5 (SPI1_SCK) | SCK | SPI Clock |
| PA6 (SPI1_MISO) | MISO | SPI Data (LoRa → STM32) |
| PA7 (SPI1_MOSI) | MOSI | SPI Data (STM32 → LoRa) |
| PC4 (GPIO OUT) | NSS | Chip Select (active LOW) |
| PC5 (GPIO OUT) | RESET | Module reset (active LOW) |
| PB0 (GPIO IN) | DIO0 | TX/RX done interrupt |
| 3.3V | VCC | Power |
| GND | GND | Common ground |

> **Note:** PA5/PA6/PA7 are shared with the onboard MEMS accelerometer on the Discovery board. PE3 is configured as GPIO output HIGH to keep the MEMS chip deselected during LoRa communication.

---

## Software Stack

- **Firmware:** STM32 HAL + FreeRTOS (generated via STM32CubeMX)
- **LoRa Driver:** [wdomski/SX1278](https://github.com/wdomski/SX1278) — STM32 HAL-compatible C driver
- **ESP32 Firmware:** Arduino framework via PlatformIO
- **Ground Station:** Python 3 (UDP receiver script)
- **Build System:** GNU Make + arm-none-eabi-gcc
- **Flash Tool:** OpenOCD via ST-Link

---

## Development Environment Setup

### Prerequisites (Fedora Linux)
```bash
sudo dnf install arm-none-eabi-gcc arm-none-eabi-newlib openocd
```

### Tools
- [STM32CubeMX 6.17+](https://www.st.com/en/development-tools/stm32cubemx.html) — Pin configuration and code generation
- [VS Code](https://code.visualstudio.com/) + Cortex-Debug extension — Firmware development
- [PlatformIO](https://platformio.org/) — ESP32 firmware development

### Clone and Build (STM32)
```bash
git clone https://github.com/YOUR_USERNAME/telemetry-gateway
cd telemetry-gateway
make
```

### Flash (STM32)
```bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program build/telemetry_gateway.elf verify reset exit"
```

### Build and Flash (ESP32)
```bash
cd esp32_wifi_gateway
pio run --target upload
```

### Run Ground Station Receiver
```bash
python3 receiver.py
```

---

## Communication Protocol

### Telemetry Packet Format
```
DEPTH:<value>,HDG:<value>,BAT:<value>,TEMP:<value>\r\n
```

### Data Priority Classes
| Class | Data | Delivery |
|---|---|---|
| HIGH | Emergency stop, depth, heading | ACK required, retransmit on failure |
| LOW | Temperature, battery voltage, sensor logs | Fire and forget |

### Link State Machine
```
         RSSI > -65dBm                RSSI < -65dBm
         Loss < 5%          ┌──────┐  Loss > 5%
    ┌────────────────────── │ GOOD │ ──────────────────────┐
    │                       └──────┘                       │
    │                                                      ▼
    │                                              ┌───────────┐
    │     RSSI > -65dBm                            │ DEGRADED  │
    │     Loss < 5%                                └─────┬─────┘
    │◄─────────────────────────────────────────────     │
    │                                              RSSI < -80dBm
    │                                              Loss > 20%
    │                                                    │
    │                                                    ▼
    │                                            ┌──────────────┐
    │         Wi-Fi restored                     │   FAILOVER   │
    └───────────────────────────────────────────►│ (LoRa only)  │
                                                 └──────────────┘
```

---

## Project Phases

| Phase | Description | Status |
|---|---|---|
| 0 | Environment validation (LED blink) | ✅ Done |
| 1 | UART pass-through + ESP32 Wi-Fi UDP | ✅ Done |
| 2 | SX1278 LoRa SPI driver integration | 🔄 In progress |
| 3 | FreeRTOS task architecture | ⬜ Pending |
| 4 | Communication Manager state machine + QoS | ⬜ Pending |
| 5 | SD card black box logger (FatFS) | ⬜ Pending |

---
## Current 

LoRa driver integrated and building clean
Hardware wired up
Next step: set up ESP32 as LoRa receiver, then verify end-to-end LoRa transmission
---

## Author

**Muhammed Elabd**
Mechatronics Engineering Graduate — HCMUT (Bach Khoa), Ho Chi Minh City
[github.com/Little3gy](https://github.com/Little3gy)