#pragma once
#include <stdint.h>
#include "../../include/config.h"

// Reads the resistor-based MOD_ID pin on boot to identify the attached add-on.
// Ref: docs/hardware-specs.md §3
class ModuleIdentifier {
public:
    // Perform the ADC read and classify the module. Call once in setup().
    static uint8_t detect();

    // Return the last detected module ID without re-reading.
    static uint8_t current();

    // Human-readable label for serial debug output.
    static const char* label(uint8_t modId);

private:
    static uint8_t _detected;
};
