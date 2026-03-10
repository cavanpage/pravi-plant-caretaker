#pragma once
#include <stdint.h>
#include "../../include/config.h"

// Circular-buffer moving average filter.
// Stabilizes ADC readings over long Cat6 cable runs exposed to EMI.
class MovingAverage {
public:
    MovingAverage();

    // Push a new raw ADC sample and return the current filtered average.
    float update(int raw);

    // Current filtered value without pushing a new sample.
    float value() const;

    // Reset all samples to zero.
    void reset();

private:
    int      _samples[FILTER_WINDOW_SIZE];
    uint8_t  _index;
    int32_t  _sum;
    bool     _full;
};
