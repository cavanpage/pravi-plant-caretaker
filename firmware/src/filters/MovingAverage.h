#pragma once
#include <stdint.h>
#include "../../include/config.h"

// Circular-buffer moving average filter.
// Stabilizes ADC readings over long Cat6 cable runs exposed to EMI.
class MovingAverage {
public:
    explicit MovingAverage(uint8_t windowSize = FILTER_WINDOW_SIZE);

    // Push a new raw ADC sample and return the current filtered average.
    float update(int raw);

    // Current filtered value without pushing a new sample.
    float value() const;

    // Reset all samples to zero.
    void reset();

private:
    uint8_t  _size;
    int      _samples[FILTER_WINDOW_SIZE];
    uint8_t  _index;
    long     _sum;
    bool     _full;
};
