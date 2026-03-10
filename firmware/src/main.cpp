#include <Arduino.h>

#include "filters/MovingAverage.h"
#include "modules/ModuleIdentifier.h"
#include "sensors/MoistureSensor.h"
#include "actuators/Pump.h"
#include "actuators/Solenoid.h"
#include "safety/SafetyInterlock.h"
#include "ui/StatusLED.h"
#include "state_machine/StateMachine.h"
#include "../include/config.h"

// ---------------------------------------------------------------------------
// Peripheral instances
// ---------------------------------------------------------------------------
MoistureSensor  moistureSensor;
Pump            pump;
Solenoid        solenoid;
SafetyInterlock safety;
StatusLED       statusLed;

StateMachine    fsm(moistureSensor, pump, solenoid, safety, statusLed);

// ---------------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial) {}   // Wait for serial monitor on native USB

    Serial.println("=== Pravi Plant Caretaker — Booting ===");

    // Configure 12-bit ADC resolution (SAM3X8E)
    analogReadResolution(ADC_RESOLUTION);

    // Init peripherals
    safety.begin();
    pump.begin();
    solenoid.begin();
    statusLed.begin();

    // Detect attached module via MOD_ID resistor
    uint8_t mod = ModuleIdentifier::detect();

    // Attach flow sensor ISR only when Direct-Line kit is present
    if (mod == MODID_DIRECT_LINE) {
        attachInterrupt(digitalPinToInterrupt(PIN_FLOW_SENSOR),
                        Solenoid::pulseCounter,
                        RISING);
        Serial.println("[Main] Flow sensor ISR attached");
    }

    // Start the state machine
    fsm.begin();

    Serial.println("=== Boot complete ===");
}

// ---------------------------------------------------------------------------
// loop — non-blocking, high-frequency poll
// ---------------------------------------------------------------------------
void loop() {
    fsm.tick();
}
