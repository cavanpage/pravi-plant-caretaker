#include "SafetyInterlock.h"
#include <Arduino.h>

SafetyInterlock::SafetyInterlock() : _actuatorRunning(false) {}

void SafetyInterlock::begin() {
    pinMode(RESERVOIR_LOW_PIN, INPUT_PULLUP);
}

bool SafetyInterlock::check() const {
    if (!isReservoirOk()) {
        Serial.println("[Safety] BLOCKED — reservoir low");
        return false;
    }
    if (!isActuatorIdle()) {
        Serial.println("[Safety] BLOCKED — actuator already running");
        return false;
    }
    return true;
}

bool SafetyInterlock::isReservoirOk() const {
    // XKC-Y25: LOW output = liquid detected at sensor level (reservoir full enough)
    // HIGH output = no liquid = reservoir empty
    return digitalRead(RESERVOIR_LOW_PIN) == LOW;
}

bool SafetyInterlock::isActuatorIdle() const {
    return !_actuatorRunning;
}

void SafetyInterlock::setActuatorRunning(bool running) {
    _actuatorRunning = running;
}
