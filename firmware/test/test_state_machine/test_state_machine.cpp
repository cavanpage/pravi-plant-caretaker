#include <unity.h>
#include "Arduino.h"
#include "sensors/MoistureSensor.h"
#include "actuators/Pump.h"
#include "actuators/Solenoid.h"
#include "safety/SafetyInterlock.h"
#include "ui/StatusLED.h"
#include "modules/ModuleIdentifier.h"
#include "state_machine/StateMachine.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Drive the sensor filter to a stable reading before ticking the FSM.
static void set_moisture(int adc) {
    mock_analog_values[PIN_SIG_A] = adc;
}

// Prime filter with a given ADC value, then advance mock time by one poll
// interval and tick the FSM.
static void prime_sensor_and_tick(StateMachine& fsm, int adc, int cycles = FILTER_WINDOW_SIZE) {
    set_moisture(adc);
    for (int i = 0; i < cycles; i++) {
        mock_millis_value += SENSOR_POLL_INTERVAL_MS;
        fsm.tick();
    }
}

// Configure MOD_ID pin so ModuleIdentifier::detect() returns the given module.
static void set_module(uint8_t mod) {
    switch (mod) {
        case MODID_MOISTURE:
            mock_analog_values[PIN_MOD_ID] = (MODID_MOISTURE_LOW + MODID_MOISTURE_HIGH) / 2;
            break;
        case MODID_RESERVOIR:
            mock_analog_values[PIN_MOD_ID] = (MODID_RESERVOIR_LOW + MODID_RESERVOIR_HIGH) / 2;
            break;
        case MODID_DIRECT_LINE:
            mock_analog_values[PIN_MOD_ID] = (MODID_DIRECTLINE_LOW + MODID_DIRECTLINE_HIGH) / 2;
            break;
        default:
            mock_analog_values[PIN_MOD_ID] = 0;
    }
    ModuleIdentifier::detect();
}

// ---------------------------------------------------------------------------
// Fixtures
// ---------------------------------------------------------------------------
static MoistureSensor  sensor;
static Pump            pump;
static Solenoid        solenoid;
static SafetyInterlock safety;
static StatusLED       led;

void setUp() {
    mock_reset();
    // Reservoir level sensor: LOW = liquid present (safe to actuate)
    mock_digital_values[RESERVOIR_LOW_PIN] = LOW;
    sensor  = MoistureSensor();
    pump    = Pump();
    solenoid = Solenoid();
    safety  = SafetyInterlock();
    led     = StatusLED();
    safety.begin();
    pump.begin();
    solenoid.begin();
    led.begin();
}

void tearDown() {}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

void test_boots_to_idle_when_moisture_ok() {
    set_module(MODID_MOISTURE);
    StateMachine fsm(sensor, pump, solenoid, safety, led);
    fsm.begin();
    prime_sensor_and_tick(fsm, 2048);   // well within OK range
    TEST_ASSERT_EQUAL(SystemState::IDLE, fsm.state());
}

void test_transitions_idle_to_warning_when_dry() {
    set_module(MODID_MOISTURE);
    StateMachine fsm(sensor, pump, solenoid, safety, led);
    fsm.begin();
    // DRY: below MOISTURE_DRY, above MOISTURE_CRITICAL
    prime_sensor_and_tick(fsm, MOISTURE_DRY - 100);
    TEST_ASSERT_EQUAL(SystemState::WARNING, fsm.state());
}

void test_transitions_idle_to_critical_when_very_dry() {
    set_module(MODID_MOISTURE);
    StateMachine fsm(sensor, pump, solenoid, safety, led);
    fsm.begin();
    prime_sensor_and_tick(fsm, MOISTURE_CRITICAL - 100);
    TEST_ASSERT_EQUAL(SystemState::CRITICAL, fsm.state());
}

void test_returns_to_idle_when_moisture_recovers() {
    set_module(MODID_MOISTURE);
    StateMachine fsm(sensor, pump, solenoid, safety, led);
    fsm.begin();

    // Drive to WARNING
    prime_sensor_and_tick(fsm, MOISTURE_DRY - 100);
    TEST_ASSERT_EQUAL(SystemState::WARNING, fsm.state());

    // Recover moisture
    prime_sensor_and_tick(fsm, 2048);
    TEST_ASSERT_EQUAL(SystemState::IDLE, fsm.state());
}

void test_led_green_in_idle() {
    set_module(MODID_MOISTURE);
    StateMachine fsm(sensor, pump, solenoid, safety, led);
    fsm.begin();
    prime_sensor_and_tick(fsm, 2048);
    TEST_ASSERT_EQUAL(LEDState::GREEN, led.current());
}

void test_led_yellow_in_warning() {
    set_module(MODID_MOISTURE);
    StateMachine fsm(sensor, pump, solenoid, safety, led);
    fsm.begin();
    prime_sensor_and_tick(fsm, MOISTURE_DRY - 100);
    TEST_ASSERT_EQUAL(LEDState::YELLOW, led.current());
}

void test_led_red_in_critical() {
    set_module(MODID_MOISTURE);
    StateMachine fsm(sensor, pump, solenoid, safety, led);
    fsm.begin();
    prime_sensor_and_tick(fsm, MOISTURE_CRITICAL - 100);
    TEST_ASSERT_EQUAL(LEDState::RED, led.current());
}

void test_fault_when_reservoir_empty() {
    set_module(MODID_RESERVOIR);
    // Simulate empty reservoir — level sensor HIGH
    mock_digital_values[RESERVOIR_LOW_PIN] = HIGH;

    StateMachine fsm(sensor, pump, solenoid, safety, led);
    fsm.begin();

    // Drive to critical so a water cycle is attempted
    prime_sensor_and_tick(fsm, MOISTURE_CRITICAL - 100);

    // Interlock should have blocked actuation → FAULT
    TEST_ASSERT_EQUAL(SystemState::FAULT, fsm.state());
}

void test_wet_status_does_not_trigger_watering() {
    set_module(MODID_RESERVOIR);
    StateMachine fsm(sensor, pump, solenoid, safety, led);
    fsm.begin();
    prime_sensor_and_tick(fsm, MOISTURE_WET + 100);
    // Should be WARNING (wet), not WATERING
    TEST_ASSERT_EQUAL(SystemState::WARNING, fsm.state());
}

// ---------------------------------------------------------------------------
// Runner
// ---------------------------------------------------------------------------
int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_boots_to_idle_when_moisture_ok);
    RUN_TEST(test_transitions_idle_to_warning_when_dry);
    RUN_TEST(test_transitions_idle_to_critical_when_very_dry);
    RUN_TEST(test_returns_to_idle_when_moisture_recovers);
    RUN_TEST(test_led_green_in_idle);
    RUN_TEST(test_led_yellow_in_warning);
    RUN_TEST(test_led_red_in_critical);
    RUN_TEST(test_fault_when_reservoir_empty);
    RUN_TEST(test_wet_status_does_not_trigger_watering);
    return UNITY_END();
}
