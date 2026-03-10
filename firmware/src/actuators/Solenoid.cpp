#include "Solenoid.h"
#include <Arduino.h>

volatile uint32_t Solenoid::_pulseCount = 0;

Solenoid::Solenoid() : _open(false) {}

void Solenoid::begin() {
    pinMode(PIN_SOLENOID, OUTPUT);
    digitalWrite(PIN_SOLENOID, LOW);
    pinMode(PIN_FLOW_SENSOR, INPUT_PULLUP);
}

void Solenoid::open() {
    digitalWrite(PIN_SOLENOID, HIGH);
    _open = true;
    Serial.println("[Solenoid] OPEN");
}

void Solenoid::close() {
    digitalWrite(PIN_SOLENOID, LOW);
    _open = false;
    Serial.println("[Solenoid] CLOSED");
}

bool Solenoid::deliverVolume(float targetLiters) {
    _pulseCount = 0;
    open();

    unsigned long start      = millis();
    unsigned long stallCheck = millis();

    while (true) {
        float currentVolume = static_cast<float>(_pulseCount) / FLOW_CALIBRATION_FACTOR;

        if (currentVolume >= targetLiters) {
            close();
            Serial.print("[Solenoid] Delivered ");
            Serial.print(currentVolume, 3);
            Serial.println(" L");
            return true;
        }

        if (millis() - start > WATCHDOG_TIMEOUT_MS) {
            close();
            Serial.println("[Solenoid] WATCHDOG timeout");
            return false;
        }

        if (isStalled(stallCheck)) {
            close();
            Serial.println("[Solenoid] STALL detected — aborting");
            return false;
        }

        // Refresh stall window every STALL_CHECK_INTERVAL_MS
        if (millis() - stallCheck >= STALL_CHECK_INTERVAL_MS) {
            stallCheck = millis();
        }
    }
}

bool Solenoid::isStalled(unsigned long windowStart) const {
    if (millis() - windowStart < STALL_CHECK_INTERVAL_MS) return false;
    // Stall: valve open but fewer than minimum pulses in the window
    return (_pulseCount < STALL_PULSE_MINIMUM);
}

bool Solenoid::isOpen() const {
    return _open;
}

void Solenoid::pulseCounter() {
    _pulseCount++;
}
