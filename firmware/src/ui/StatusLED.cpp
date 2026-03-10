#include "StatusLED.h"
#include <Arduino.h>

StatusLED::StatusLED() : _current(LEDState::GREEN) {}

void StatusLED::begin() {
    pinMode(PIN_LED_GREEN,  OUTPUT);
    pinMode(PIN_LED_YELLOW, OUTPUT);
    pinMode(PIN_LED_RED,    OUTPUT);
    allOff();
}

void StatusLED::set(LEDState state) {
    allOff();
    _current = state;
    switch (state) {
        case LEDState::GREEN:  digitalWrite(PIN_LED_GREEN,  HIGH); break;
        case LEDState::YELLOW: digitalWrite(PIN_LED_YELLOW, HIGH); break;
        case LEDState::RED:    digitalWrite(PIN_LED_RED,    HIGH); break;
    }
}

void StatusLED::blink(LEDState state, uint8_t times, uint16_t periodMs) {
    for (uint8_t i = 0; i < times; i++) {
        set(state);
        delay(periodMs / 2);
        allOff();
        delay(periodMs / 2);
    }
    // Restore previous state after blinking
    set(_current);
}

LEDState StatusLED::current() const {
    return _current;
}

void StatusLED::allOff() const {
    digitalWrite(PIN_LED_GREEN,  LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_RED,    LOW);
}
