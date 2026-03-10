#pragma once
#include <stdint.h>
#include "../filters/MovingAverage.h"
#include "../../include/config.h"

enum class MoistureStatus {
    CRITICAL,   // Below MOISTURE_CRITICAL — immediate watering needed
    DRY,        // Below MOISTURE_DRY — watering needed
    OK,         // Normal range
    WET         // Above MOISTURE_WET — oversaturated, do not water
};

class MoistureSensor {
public:
    MoistureSensor();

    // Read raw ADC, push through filter, classify status. Call each poll cycle.
    void update();

    // Filtered ADC count (0–4095).
    float rawFiltered() const;

    // Moisture as a percentage (0–100%).
    float percent() const;

    // Current classified status.
    MoistureStatus status() const;

private:
    MovingAverage _filter;
    float         _filtered;
    MoistureStatus _status;

    MoistureStatus classify(float value) const;
};
