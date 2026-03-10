#include "Pump.h"
#include <Arduino.h>

Pump::Pump() : _running(false) {}

void Pump::begin() {
    pinMode(PIN_PUMP, OUTPUT);
    digitalWrite(PIN_PUMP, LOW);
}

void Pump::runCycle() {
    _running = true;
    digitalWrite(PIN_PUMP, HIGH);
    Serial.println("[Pump] ON");

    unsigned long start = millis();
    while (millis() - start < PUMP_RUN_MS) {
        if (millis() - start > WATCHDOG_TIMEOUT_MS) {
            Serial.println("[Pump] WATCHDOG — force off");
            break;
        }
    }

    digitalWrite(PIN_PUMP, LOW);
    _running = false;
    Serial.println("[Pump] OFF");
}

bool Pump::isRunning() const {
    return _running;
}
