#pragma once

// ---------------------------------------------------------------------------
// Pravi Plant Caretaker — Central Configuration
// Target: Arduino Due (Atmel SAM3X8E, ARM Cortex-M3, 3.3V logic)
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Pravi Port — Pin Assignments
// Ref: docs/hardware-specs.md
// ---------------------------------------------------------------------------

// RJ45 Port 1 signal lines
#define PIN_SIG_A           A0      // Pin 6 — Primary analog input (moisture)
#define PIN_SIG_B           A1      // Pin 7 — Secondary analog input
#define PIN_MOD_ID          A2      // Pin 8 — Module identification resistor divider

// Actuator control (via MOSFET/relay driver)
#define PIN_PUMP            7       // Reservoir kit — submersible pump
#define PIN_SOLENOID        8       // Direct-line kit — solenoid valve
#define PIN_FLOW_SENSOR     2       // Hall-effect flow sensor (interrupt-capable)

// Status LED matrix (traffic-light — Phase 1 UI)
#define PIN_LED_GREEN       22      // Healthy
#define PIN_LED_YELLOW      23      // Warning
#define PIN_LED_RED         24      // Critical

// ---------------------------------------------------------------------------
// ADC & Filtering
// ---------------------------------------------------------------------------

#define ADC_RESOLUTION      12      // 12-bit ADC → 0–4095 counts
#define FILTER_WINDOW_SIZE  16      // Moving average sample count

// ---------------------------------------------------------------------------
// Moisture Thresholds (ADC counts, 0–4095)
// Calibrate per soil type. Defaults for general potting mix.
// ---------------------------------------------------------------------------

#define MOISTURE_DRY        1200    // Below → needs watering (warning)
#define MOISTURE_CRITICAL   800     // Below → immediate water (critical)
#define MOISTURE_WET        3200    // Above → oversaturated (warning)

// ---------------------------------------------------------------------------
// Module Identification — MOD_ID resistor voltage thresholds (ADC counts)
// The Due reads a resistor divider on Pin 8 to detect the attached add-on.
// Ref: docs/hardware-specs.md §3
// ---------------------------------------------------------------------------

#define MODID_NONE          0
#define MODID_MOISTURE      1       // 1.0 kΩ  — Base Capacitive Moisture Sensor
#define MODID_RESERVOIR     2       // 4.7 kΩ  — Reservoir Kit (Pump + Level Sensor)
#define MODID_DIRECT_LINE   3       // 10.0 kΩ — Direct-Line Kit (Solenoid + Flow Meter)

// ADC count thresholds for each resistor value (10-bit window around expected)
#define MODID_MOISTURE_LOW  350
#define MODID_MOISTURE_HIGH 500
#define MODID_RESERVOIR_LOW 1600
#define MODID_RESERVOIR_HIGH 1800
#define MODID_DIRECTLINE_LOW 2600
#define MODID_DIRECTLINE_HIGH 2850

// ---------------------------------------------------------------------------
// Watering
// ---------------------------------------------------------------------------

#define WATER_TARGET_LITERS     0.5f    // Default dose per cycle (Direct-Line)
#define PUMP_RUN_MS             3000    // Default pump-on duration (Reservoir)
#define FLOW_CALIBRATION_FACTOR 7.5f    // YF-S201: pulses per liter
#define WATCHDOG_TIMEOUT_MS     15000   // Max time any actuator may run

// ---------------------------------------------------------------------------
// Safety
// ---------------------------------------------------------------------------

#define STALL_CHECK_INTERVAL_MS 2000    // How often to check for flow stall
#define STALL_PULSE_MINIMUM     5       // Min pulses expected during stall window
#define RESERVOIR_LOW_PIN       9       // XKC-Y25 level sensor digital output

// ---------------------------------------------------------------------------
// Timing
// ---------------------------------------------------------------------------

#define SENSOR_POLL_INTERVAL_MS 500     // Main loop sensor read interval
#define SERIAL_BAUD             115200
