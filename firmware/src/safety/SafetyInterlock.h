#pragma once
#include <stdint.h>
#include "../../include/config.h"

// Aggregates all pre-actuation safety checks.
// Every actuator must call check() before opening a valve or running a pump.
class SafetyInterlock {
public:
    SafetyInterlock();

    void begin();

    // Returns true if it is safe to actuate.
    bool check() const;

    // Individual condition checks (public for state machine diagnostics).
    bool isReservoirOk() const;    // Reservoir kit: XKC-Y25 level sensor HIGH
    bool isActuatorIdle() const;   // No other actuator currently running

    // Register external actuator state so the interlock can gate on it.
    void setActuatorRunning(bool running);

private:
    bool _actuatorRunning;
};
