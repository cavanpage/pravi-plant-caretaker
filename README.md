# Pravi Plant Caretaker
**A Modular, Industrial-Grade Embedded System for High-Precision Irrigation**

## Project Overview
The Pravi Plant Caretaker (derived from the Croatian word for "Proper") is a deterministic, hardwired irrigation system designed for the Arduino Due (ARM Cortex-M3). Moving away from consumer-grade wireless dependencies, Pravi prioritizes signal integrity, modular hardware expansion, and fail-safe execution. 

This repository serves as a technical showcase for bridging low-level embedded C++ with high-level systems architecture.

## Technical Documentation
* [Hardware Interface Specification](docs/hardware-specs.md) - Pinouts, electrical standards, and modularity.
* [Software Architecture & Interlocks](docs/software-logic.md) - State machine details and safety logic.

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

---

## About the Developer
Senior Software Engineer with 10 years of experience in distributed systems and microservices. This project serves as a demonstration of low-level embedded capabilities, real-time safety logic, and hardware-software integration.
