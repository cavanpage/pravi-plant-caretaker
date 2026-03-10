#include "MovingAverage.h"
#include <string.h>

MovingAverage::MovingAverage()
    : _index(0), _sum(0), _full(false)
{
    memset(_samples, 0, sizeof(_samples));
}

float MovingAverage::update(int raw) {
    _sum -= _samples[_index];
    _samples[_index] = raw;
    _sum += raw;

    _index = (_index + 1) % _size;
    if (_index == 0) _full = true;

    return value();
}

float MovingAverage::value() const {
    uint8_t count = _full ? FILTER_WINDOW_SIZE : _index;
    if (count == 0) return 0.0f;
    return static_cast<float>(_sum) / count;
}

void MovingAverage::reset() {
    memset(_samples, 0, sizeof(_samples));
    _index = 0;
    _sum   = 0;
    _full  = false;
}
