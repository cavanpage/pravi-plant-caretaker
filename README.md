# Pravi Plant Caretaker (v1.0)
**A Modular, Industrial-Grade Embedded Ecosystem for High-Precision Irrigation**

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![Platform: Arduino Due](https://img.shields.io/badge/Platform-Arduino%20Due%20(ARM%20Cortex--M3)-green.svg)

## The "Pravi" Philosophy
Derived from the Croatian word for **"Proper,"** the Pravi Plant Caretaker is an engineering-first approach to indoor horticulture. Moving away from fragile WiFi-dependent consumer gadgets, Pravi focuses on **deterministic execution**, **hardwired signal integrity**, and **modular actuation**.

This project showcases the bridge between low-level embedded C++ (Real-time safety & sensing) and high-level systems (Go-based telemetry & monitoring).

---

## Technical Architecture

### 1. High-Resolution Sensing (12-bit ADC)
Unlike standard 8-bit microcontrollers, the **Atmel SAM3X8E (ARM Cortex-M3)** provides 4096 levels of resolution. 
* **Signal Conditioning:** Implemented software-based Moving Average Filters to mitigate EMI noise introduced by 10ft+ Cat6 cable runs.
* **Capacitive Coupling:** Utilizes non-corrosive capacitive probes to ensure long-term sensor stability without soil electrolysis.

### 2. Deterministic Real-Time Control
The firmware utilizes a **Non-Blocking State Machine**. By avoiding the `delay()` function, the system maintains a high-frequency polling rate for safety interlocks, ensuring that a "Reservoir Empty" or "Leak Detected" event triggers an immediate hardware interrupt.

### 3. Modular "Add-on" Ecosystem
Pravi is designed with a standardized **RJ45/Cat6 Interface**, allowing the system to scale from a single pot to a professional-grade grow operation:
* **Mineral Module:** RS485/Modbus integration for NPK (Nitrogen, Phosphorus, Potassium) and pH monitoring.
* **Flow Module:** Hall-effect pulse counting for precise mL dosing.
* **Safety Module:** Non-contact liquid level sensing (XKC-Y25) and floor-leak detection.



---

## Engineering Deep Dive

### Interrupt-Driven Flow Verification
For professional "Direct-Line" (Hose) setups, accuracy is paramount. Pravi uses hardware interrupts to track water volume in real-time.

```cpp
// ISR for Hall-Effect Flow Sensor
void pulseCounter() {
    pulseCount++;
}

// Logic to prevent "Dry-Run" or "Solenoid Failure"
void executePraviCycle(float targetLiters) {
    if (!checkWaterAvailability()) return; 
    
    openActuator(); // Relay for Pump or Solenoid for Hose
    while (currentLiters < targetLiters && !timeoutReached()) {
        currentLiters = calculateVolume(pulseCount);
        if (stallDetected()) handleCriticalError(); 
    }
    closeActuator();
}
