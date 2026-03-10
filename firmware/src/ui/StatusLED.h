#pragma once
#include "../../include/config.h"

// Traffic-light status LED matrix — Phase 1 local UI.
// GREEN = healthy, YELLOW = warning, RED = critical.
enum class LEDState {
    GREEN,
    YELLOW,
    RED
};

class StatusLED {
public:
    StatusLED();

    void begin();

    // Set the active LED. Turns off all others.
    void set(LEDState state);

    // Blink the current LED n times (blocking). Used for error codes.
    void blink(LEDState state, uint8_t times, uint16_t periodMs = 300);

    LEDState current() const;

private:
    LEDState _current;

    void allOff() const;
};
