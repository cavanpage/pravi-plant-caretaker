#pragma once

#include "../sensors/MoistureSensor.h"
#include "../actuators/Pump.h"
#include "../actuators/Solenoid.h"
#include "../safety/SafetyInterlock.h"
#include "../ui/StatusLED.h"
#include "../modules/ModuleIdentifier.h"
#include "../../include/config.h"

// ---------------------------------------------------------------------------
// System States
// ---------------------------------------------------------------------------
enum class SystemState {
    BOOT,           // Module detection, hardware init
    IDLE,           // Sensors polling, no action needed
    WATERING,       // Actuator active — delivering water
    WARNING,        // Moisture outside nominal range (dry/wet) — alerting
    CRITICAL,       // Moisture critically low — initiating emergency water
    FAULT           // Safety interlock tripped or actuator failure
};

// ---------------------------------------------------------------------------
// StateMachine
// Non-blocking state machine. tick() is called every SENSOR_POLL_INTERVAL_MS.
// ---------------------------------------------------------------------------
class StateMachine {
public:
    StateMachine(MoistureSensor& sensor,
                 Pump&            pump,
                 Solenoid&        solenoid,
                 SafetyInterlock& safety,
                 StatusLED&       led);

    // Called once in setup() after all peripherals are initialized.
    void begin();

    // Called every loop iteration. Drives state transitions and actions.
    void tick();

    SystemState state() const;

private:
    MoistureSensor&  _sensor;
    Pump&            _pump;
    Solenoid&        _solenoid;
    SafetyInterlock& _safety;
    StatusLED&       _led;

    SystemState      _state;
    unsigned long    _lastPollMs;

    void transitionTo(SystemState next);
    void handleIdle();
    void handleWarning();
    void handleCritical();
    void handleWatering();
    void handleFault();

    void logState() const;
    void updateLED() const;
};
