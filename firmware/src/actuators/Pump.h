#pragma once
#include "../../include/config.h"

// Controls the submersible pump used in the Reservoir Kit.
// Pump runs for a fixed duration (PUMP_RUN_MS) per watering cycle.
class Pump {
public:
    Pump();

    void begin();

    // Activate the pump for PUMP_RUN_MS milliseconds (blocking with watchdog).
    void runCycle();

    bool isRunning() const;

private:
    bool _running;
};
