# Software Architecture & Interlocks

## State Machine

Pravi's firmware is built on a non-blocking finite state machine. `StateMachine::tick()` is called every `SENSOR_POLL_INTERVAL_MS` from the Arduino `loop()` and drives all transitions. No blocking delays exist in the main loop — only inside actuator methods during active delivery cycles.

```
BOOT → IDLE ←→ WARNING ←→ WATERING
                  ↕              ↓
               CRITICAL       FAULT
```

| State | Condition | LED |
|:---|:---|:---|
| IDLE | Moisture OK | GREEN |
| WARNING | Moisture DRY or WET | YELLOW |
| CRITICAL | Moisture below critical threshold | RED |
| WATERING | Actuator active | YELLOW |
| FAULT | Interlock tripped or delivery failure | RED (blink) |

---

## Moving Average Filter

All ADC readings pass through a circular-buffer moving average before classification. This eliminates EMI-induced spikes common over long Cat6 cable runs.

- Window size: `FILTER_WINDOW_SIZE` (default 16 samples)
- Sum type: `int32_t` — safe for 12-bit ADC (`16 × 4095 = 65,520`)
- Pre-fill handling: denominator uses actual sample count until the buffer is full

---

## Module Identification

On boot, the firmware reads a resistor divider on MOD_ID (Pin 8) to detect the attached add-on. This determines which actuator path is used during a watering cycle.

| Resistor | Module | Actuator |
|:---|:---|:---|
| 1.0 kΩ | Base Moisture Sensor | None (sense only) |
| 4.7 kΩ | Reservoir Kit | Pump (timed run) |
| 10.0 kΩ | Direct-Line Kit | Solenoid + flow meter |

---

## Volumetric Flow Control (Direct-Line Kit)

The YF-S201 hall-effect sensor triggers a hardware interrupt on every pulse. `Solenoid::deliverVolume()` counts pulses against the calibration factor to deliver a precise volume, with two independent abort conditions:

```cpp
// ISR — attached in main setup() only when Direct-Line kit is detected
void Solenoid::pulseCounter() {
    _pulseCount++;
}

// Deliver targetLiters. Returns true on success, false on watchdog/stall.
bool Solenoid::deliverVolume(float targetLiters) {
    _pulseCount = 0;
    open();

    while (true) {
        float vol = _pulseCount / FLOW_CALIBRATION_FACTOR;
        if (vol >= targetLiters)              { close(); return true; }
        if (millis() - start > WATCHDOG_TIMEOUT_MS) { close(); return false; }
        if (isStalled())                      { close(); return false; }
    }
}
```

**Abort conditions:**

| Condition | Trigger | Action |
|:---|:---|:---|
| Target reached | `currentVolume >= targetLiters` | Close solenoid, return success |
| Watchdog | Elapsed > `WATCHDOG_TIMEOUT_MS` | Force close, return failure → FAULT |
| Stall | < `STALL_PULSE_MINIMUM` pulses in `STALL_CHECK_INTERVAL_MS` window | Force close, return failure → FAULT |

---

## Safety Interlock

`SafetyInterlock::check()` must return `true` before any actuator is activated. It gates on:

1. **Reservoir level** — XKC-Y25 non-contact sensor on `RESERVOIR_LOW_PIN`. LOW = liquid present.
2. **Actuator idle** — prevents concurrent actuation.

All thresholds, timeouts, and pin assignments are centralised in `include/config.h`.
