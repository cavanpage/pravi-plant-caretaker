#pragma once
#include "../../include/config.h"

// Controls the 12V solenoid valve and tracks volumetric flow via the
// YF-S201 hall-effect sensor for the Direct-Line Kit.
// The flow sensor ISR (pulseCounter) must be attached in main setup().
class Solenoid {
public:
    Solenoid();

    void begin();

    // Deliver exactly targetLiters. Returns true on success, false on error.
    bool deliverVolume(float targetLiters = WATER_TARGET_LITERS);

    bool isOpen() const;

    // ISR — call from the hardware interrupt attached to PIN_FLOW_SENSOR.
    static void IRAM_ATTR pulseCounter();

private:
    bool _open;

    void open();
    void close();
    bool isStalled(unsigned long windowStart) const;

    static volatile uint32_t _pulseCount;
};
