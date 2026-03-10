# Pravi Plant Caretaker
**A Modular, Industrial-Grade Embedded System for High-Precision Irrigation**

## Project Overview
The Pravi Plant Caretaker (derived from the Croatian word for "Proper") is a deterministic, hardwired irrigation system designed for the Arduino Due (ARM Cortex-M3). Moving away from consumer-grade wireless dependencies, Pravi prioritizes signal integrity, modular hardware expansion, and fail-safe execution.

This repository serves as a technical showcase for bridging low-level embedded C++ with high-level systems architecture.

## Technical Documentation
* [Hardware Interface Specification](docs/hardware-specs.md) - Pinouts, electrical standards, and modularity.
* [Software Architecture & Interlocks](docs/software-logic.md) - State machine details and safety logic.
* [AI Vision Add-on Specification](docs/ai-vision-addon.md) - Phase 4 computer vision integration.

---

## Getting Started

### Prerequisites

| Tool | Purpose | Install |
|:---|:---|:---|
| Python 3 | Required by PlatformIO | Pre-installed on macOS; [python.org](https://python.org) otherwise |
| pipx | Isolated package installer | `brew install pipx && pipx ensurepath` |
| [PlatformIO CLI](https://docs.platformio.org/en/latest/core/installation/index.html) | Build & flash firmware | `pipx install platformio` |
| Arduino Due board support | Installed automatically by PlatformIO on first build | — |

### Repository Structure

```
firmware/
├── include/config.h          # All pin definitions, thresholds, constants
├── src/
│   ├── main.cpp              # Entry point (setup / loop)
│   ├── filters/              # Moving average ADC filter
│   ├── modules/              # MOD_ID hardware detection
│   ├── sensors/              # Moisture sensor
│   ├── actuators/            # Pump + Solenoid (with flow metering)
│   ├── safety/               # Pre-actuation interlock
│   ├── ui/                   # Traffic-light LED matrix
│   └── state_machine/        # Core FSM
├── test/
│   ├── mocks/Arduino.h       # HAL stub for host-side testing
│   ├── test_moving_average/
│   ├── test_moisture_sensor/
│   └── test_state_machine/
└── scripts/test.sh           # Test runner
```

### Running Tests (No Hardware Required)

All business logic is unit-tested against a mocked Arduino HAL and runs natively on your machine.

```bash
cd firmware

# Run all suites
./scripts/test.sh

# Run a specific suite
./scripts/test.sh moving_average
./scripts/test.sh moisture_sensor
./scripts/test.sh state_machine
```

Or directly via PlatformIO:

```bash
cd firmware
pio test -e native
```

### Building & Flashing to Arduino Due

#### 1. Connect the Board

The Arduino Due has two USB ports. Use the **Native USB port** (closest to the DC barrel jack) for programming.

> **Note:** If PlatformIO cannot find the port, check `System Information > USB` (macOS) or run `ls /dev/cu.*` to identify the correct device. The Due typically appears as `/dev/cu.usbmodem<id>`.

#### 2. Upload

```bash
cd firmware

# Compile only (no board required)
pio run -e due

# Compile and upload
pio run -e due --target upload

# Open serial monitor at 115200 baud to observe state machine output
pio device monitor
```

Expected serial output on a healthy boot:

```
=== Pravi Plant Caretaker — Booting ===
[ModuleIdentifier] MOD_ID ADC=425 → Reservoir Kit
[StateMachine] → IDLE | moisture=62.3%
```

#### 3. Troubleshooting Upload

| Symptom | Fix |
|:---|:---|
| `No device found on ...` | Ensure Native USB port is used, not Programming port |
| `OSError: [Errno 16] Resource busy` | Close any open serial monitor before uploading |
| Board not detected at all | Press the **ERASE** button on the Due, then **RESET**, then retry upload |

---

### Hardware Setup

> **Safety first:** The Arduino Due runs at **3.3V logic**. Never connect 5V sensor outputs directly — use a voltage divider. Never drive pumps or solenoids directly from GPIO — use a MOSFET driver or opto-isolated relay with a 1N4007 flyback diode across the load.

#### Pravi Port Pinout (RJ45 / Cat6)

| RJ45 Pin | Signal | Voltage | Arduino Due Pin |
|:---|:---|:---|:---|
| 1 | VCC_LOGIC | 3.3V | 3.3V rail |
| 2 | VCC_ACTUATOR | 12V | External 12V supply |
| 3 | GND | 0V | GND |
| 4 | I2C_SDA | 3.3V | SDA1 (Pin 20) |
| 5 | I2C_SCL | 3.3V | SCL1 (Pin 21) |
| 6 | SIG_A | 0–3.3V | A0 (moisture signal) |
| 7 | SIG_B | 0–3.3V | A1 (auxiliary signal) |
| 8 | MOD_ID | 0–3.3V | A2 (module ID resistor) |

#### Status LED Matrix (All Kits)

Wire three LEDs with 220Ω current-limiting resistors between each GPIO pin and GND.

| Colour | Arduino Due Pin | Meaning |
|:---|:---|:---|
| Green | Pin 22 | Healthy — moisture OK |
| Yellow | Pin 23 | Warning — soil dry or oversaturated |
| Red | Pin 24 | Critical — immediate attention needed / fault |

```
Pin 22 ──[220Ω]──[GREEN LED]── GND
Pin 23 ──[220Ω]──[YELLOW LED]── GND
Pin 24 ──[220Ω]──[RED LED]── GND
```

#### Category A: Reservoir Kit Wiring

| Component | Connection | Arduino Due Pin |
|:---|:---|:---|
| Capacitive moisture sensor | SIG_A output → A0 | A0 |
| MOD_ID resistor (4.7 kΩ) | Between A2 and GND | A2 |
| Submersible pump | Via MOSFET/relay gate | Pin 7 |
| XKC-Y25 level sensor | Signal output → Pin 9 | Pin 9 |

**Pump wiring (MOSFET driver):**
```
Pin 7 ──[MOSFET gate]──[Pump+] ── 12V supply
                      [Pump-] ──[1N4007 diode]── GND
```

The 1N4007 flyback diode is placed across the pump terminals (cathode to +, anode to −) to suppress the inductive voltage spike when the pump switches off.

**XKC-Y25 level sensor:**
```
XKC-Y25 VCC  → 5V (or 3.3V — check sensor model)
XKC-Y25 GND  → GND
XKC-Y25 OUT  → Pin 9   (LOW = liquid detected, HIGH = reservoir empty)
```

> If your XKC-Y25 is a 5V model, use a 10kΩ/20kΩ voltage divider on the OUT line before connecting to Pin 9.

#### Category B: Direct-Line Kit Wiring

| Component | Connection | Arduino Due Pin |
|:---|:---|:---|
| Capacitive moisture sensor | SIG_A output → A0 | A0 |
| MOD_ID resistor (10.0 kΩ) | Between A2 and GND | A2 |
| 12V solenoid valve | Via MOSFET/relay gate | Pin 8 |
| YF-S201 flow sensor | Signal output → Pin 2 | Pin 2 (interrupt) |

**Solenoid wiring (MOSFET driver):**
```
Pin 8 ──[MOSFET gate]──[Solenoid+] ── 12V supply
                       [Solenoid-] ──[1N4007 diode]── GND
```

**YF-S201 flow sensor:**
```
YF-S201 VCC    → 5V
YF-S201 GND    → GND
YF-S201 Signal → Pin 2   (pulse output — hardware interrupt, 3.3V-safe on Due)
```

### Calibration

All thresholds and pin assignments live in one place: [`firmware/include/config.h`](firmware/include/config.h).

Key values to tune per soil type:

```cpp
#define MOISTURE_DRY      1200   // ADC count below which watering starts
#define MOISTURE_CRITICAL  800   // ADC count for emergency watering
#define MOISTURE_WET      3200   // ADC count above which watering is suppressed
```

---

## Core Engineering Principles

### 1. Deterministic Real-Time Control
The firmware is built on a non-blocking state machine architecture. By eliminating thread-blocking delays, the system maintains a high-frequency polling rate for safety interlocks. This ensures that critical events—such as leak detection or reservoir depletion—trigger immediate hardware responses even during active irrigation cycles.

### 2. High-Resolution Signal Processing
Utilizing the 12-bit ADC of the Atmel SAM3X8E, Pravi achieves 4096 levels of resolution for moisture sensing.

* **Noise Mitigation:** Implemented software-based Moving Average Filters to stabilize analog readings affected by electromagnetic interference (EMI) over 10ft+ Cat6 cable runs.
* **Capacitive Sensing:** Employs non-contact capacitive probes to eliminate the corrosion and soil electrolysis common in resistive sensing kits.

---

## Modular Add-on Ecosystem

Pravi is designed to be hardware-agnostic regarding the water source.

### Category A: The Reservoir Kit (Off-Grid)
* **Actuation:** Low-voltage submersible pump.
* **Safety:** Non-contact liquid level sensing (XKC-Y25) mounted to the reservoir exterior.

### Category B: The Direct-Line Kit (Professional/Indoor)
* **Actuation:** 12V DC Solenoid Valve.
* **Metering:** Hall-effect flow sensor (YF-S201) utilizing hardware interrupts for precise volumetric dosing.

---

## Project Roadmap

### Phase 1: Local Hardware Stack (MVP)
- Hardwired Sensor Loop (Cat6/RJ45) implementation.
- 12-bit ADC filtering and calibration algorithms.
- Local UI: Traffic-light status LED matrix (Status/Warning/Critical).

### Phase 2: Environmental Intelligence
- **Evapotranspiration Adjustment:** Dynamic watering thresholds based on ambient temperature (DHT22).
- **Mineral Tracking:** RS485/Modbus integration for 7-in-1 Soil NPK sensors.
- **Universal Motherboard:** PCB design for the Arduino Due to break out modular RJ45 ports.

### Phase 3: The Full-Stack Bridge
- **Go-Gateway:** A high-level service to consume Serial telemetry and serve a local web dashboard.
- **SMTP Notification Engine:** Automated email alerts for reservoir refills and system errors.

### Phase 4: AI Visual Intelligence
- **Plant Health Monitor:** Periodic camera captures analyzed by a vision AI model to detect disease, pests, yellowing, and wilting.
- **Growth Tracking:** Time-lapse comparison to identify stunted growth or abnormal changes over time.
- **Sensor Fusion Diagnosis:** Correlate visual findings with live sensor data (moisture, NPK, temperature) for higher-confidence plant health assessments.
- **Automated Response:** AI-driven watering and nutrient adjustments triggered by visual diagnoses.

See [AI Vision Add-on Specification](docs/ai-vision-addon.md) for full architecture and integration details.

---

## About the Developer
Senior Software Engineer with 10 years of experience in distributed systems and microservices. This project serves as a demonstration of low-level embedded capabilities, real-time safety logic, and hardware-software integration.
