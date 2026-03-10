#include "StateMachine.h"
#include <Arduino.h>

StateMachine::StateMachine(MoistureSensor& sensor,
                           Pump&            pump,
                           Solenoid&        solenoid,
                           SafetyInterlock& safety,
                           StatusLED&       led)
    : _sensor(sensor), _pump(pump), _solenoid(solenoid),
      _safety(safety), _led(led),
      _state(SystemState::BOOT), _lastPollMs(0)
{}

void StateMachine::begin() {
    transitionTo(SystemState::IDLE);
}

void StateMachine::tick() {
    unsigned long now = millis();
    if (now - _lastPollMs < SENSOR_POLL_INTERVAL_MS) return;
    _lastPollMs = now;

    _sensor.update();

    switch (_state) {
        case SystemState::IDLE:     handleIdle();     break;
        case SystemState::WARNING:  handleWarning();  break;
        case SystemState::CRITICAL: handleCritical(); break;
        case SystemState::WATERING: handleWatering(); break;
        case SystemState::FAULT:    handleFault();    break;
        default: break;
    }
}

// ---------------------------------------------------------------------------
// State Handlers
// ---------------------------------------------------------------------------

void StateMachine::handleIdle() {
    switch (_sensor.status()) {
        case MoistureStatus::CRITICAL:
            transitionTo(SystemState::CRITICAL);
            break;
        case MoistureStatus::DRY:
        case MoistureStatus::WET:
            transitionTo(SystemState::WARNING);
            break;
        case MoistureStatus::OK:
            // Stay idle — nothing to do
            break;
    }
}

void StateMachine::handleWarning() {
    switch (_sensor.status()) {
        case MoistureStatus::CRITICAL:
            transitionTo(SystemState::CRITICAL);
            break;
        case MoistureStatus::OK:
            transitionTo(SystemState::IDLE);
            break;
        case MoistureStatus::DRY:
            // Only water if an actuator module is attached
            if (ModuleIdentifier::current() == MODID_RESERVOIR ||
                ModuleIdentifier::current() == MODID_DIRECT_LINE) {
                transitionTo(SystemState::WATERING);
            }
            // else: sense-only module — hold WARNING, no actuator to drive
            break;
        case MoistureStatus::WET:
            // Oversaturated — hold warning, do not water
            break;
    }
}

void StateMachine::handleCritical() {
    if (ModuleIdentifier::current() == MODID_RESERVOIR ||
        ModuleIdentifier::current() == MODID_DIRECT_LINE) {
        transitionTo(SystemState::WATERING);
    }
    // else: sense-only module — hold CRITICAL until actuator is connected
}

void StateMachine::handleWatering() {
    if (!_safety.check()) {
        transitionTo(SystemState::FAULT);
        return;
    }

    bool success = false;
    uint8_t mod  = ModuleIdentifier::current();

    _safety.setActuatorRunning(true);

    if (mod == MODID_RESERVOIR) {
        _pump.runCycle();
        success = true;
    } else if (mod == MODID_DIRECT_LINE) {
        success = _solenoid.deliverVolume(WATER_TARGET_LITERS);
    } else {
        Serial.println("[StateMachine] No actuator module — cannot water");
        success = false;
    }

    _safety.setActuatorRunning(false);

    if (success) {
        transitionTo(SystemState::IDLE);
    } else {
        transitionTo(SystemState::FAULT);
    }
}

void StateMachine::handleFault() {
    // Hold in FAULT state. Blink red. Require manual reset.
    _led.blink(LEDState::RED, 3, 400);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void StateMachine::transitionTo(SystemState next) {
    _state = next;
    updateLED();
    logState();
}

void StateMachine::updateLED() const {
    switch (_state) {
        case SystemState::IDLE:     _led.set(LEDState::GREEN);  break;
        case SystemState::WARNING:  _led.set(LEDState::YELLOW); break;
        case SystemState::CRITICAL:
        case SystemState::FAULT:    _led.set(LEDState::RED);    break;
        case SystemState::WATERING: _led.set(LEDState::YELLOW); break;
        default: break;
    }
}

void StateMachine::logState() const {
    const char* label = "UNKNOWN";
    switch (_state) {
        case SystemState::BOOT:     label = "BOOT";     break;
        case SystemState::IDLE:     label = "IDLE";     break;
        case SystemState::WARNING:  label = "WARNING";  break;
        case SystemState::CRITICAL: label = "CRITICAL"; break;
        case SystemState::WATERING: label = "WATERING"; break;
        case SystemState::FAULT:    label = "FAULT";    break;
    }
    Serial.print("[StateMachine] → ");
    Serial.print(label);
    Serial.print(" | moisture=");
    Serial.print(_sensor.percent(), 1);
    Serial.println("%");
}

SystemState StateMachine::state() const {
    return _state;
}
