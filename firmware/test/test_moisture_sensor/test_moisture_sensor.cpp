#include <unity.h>
#include "Arduino.h"
#include "sensors/MoistureSensor.h"

void setUp()    { mock_reset(); }
void tearDown() {}

// Helper — drive the sensor through a full filter window at a fixed ADC value,
// then call update() one more time (the value under test).
static MoistureStatus prime_and_read(int stable_adc, int final_adc) {
    MoistureSensor s;
    mock_analog_values[PIN_SIG_A] = stable_adc;
    for (int i = 0; i < FILTER_WINDOW_SIZE - 1; i++) s.update();
    mock_analog_values[PIN_SIG_A] = final_adc;
    s.update();
    return s.status();
}

// ---------------------------------------------------------------------------
// Status classification
// ---------------------------------------------------------------------------

void test_status_ok_mid_range() {
    MoistureSensor s;
    mock_analog_values[PIN_SIG_A] = 2048;
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) s.update();
    TEST_ASSERT_EQUAL(MoistureStatus::OK, s.status());
}

void test_status_dry_below_threshold() {
    // Set ADC just below MOISTURE_DRY but above MOISTURE_CRITICAL
    int dry_adc = MOISTURE_DRY - 50;
    MoistureSensor s;
    mock_analog_values[PIN_SIG_A] = dry_adc;
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) s.update();
    TEST_ASSERT_EQUAL(MoistureStatus::DRY, s.status());
}

void test_status_critical_below_threshold() {
    int crit_adc = MOISTURE_CRITICAL - 50;
    MoistureSensor s;
    mock_analog_values[PIN_SIG_A] = crit_adc;
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) s.update();
    TEST_ASSERT_EQUAL(MoistureStatus::CRITICAL, s.status());
}

void test_status_wet_above_threshold() {
    int wet_adc = MOISTURE_WET + 50;
    MoistureSensor s;
    mock_analog_values[PIN_SIG_A] = wet_adc;
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) s.update();
    TEST_ASSERT_EQUAL(MoistureStatus::WET, s.status());
}

// ---------------------------------------------------------------------------
// Percent output
// ---------------------------------------------------------------------------

void test_percent_at_zero_adc() {
    MoistureSensor s;
    mock_analog_values[PIN_SIG_A] = 0;
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) s.update();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, s.percent());
}

void test_percent_at_max_adc() {
    MoistureSensor s;
    mock_analog_values[PIN_SIG_A] = 4095;
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) s.update();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, s.percent());
}

void test_percent_clamped_never_exceeds_100() {
    MoistureSensor s;
    mock_analog_values[PIN_SIG_A] = 4095;
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) s.update();
    TEST_ASSERT_LESS_OR_EQUAL(100.0f, s.percent());
}

// ---------------------------------------------------------------------------
// Filter smoothing — a spike shouldn't immediately flip status
// ---------------------------------------------------------------------------

void test_single_spike_does_not_flip_to_critical() {
    // Prime with healthy mid-range, then inject one critical spike
    MoistureStatus result = prime_and_read(2048, MOISTURE_CRITICAL - 200);
    // With 15 samples at 2048 and 1 sample at ~600, average stays above DRY
    TEST_ASSERT_NOT_EQUAL(MoistureStatus::CRITICAL, result);
}

// ---------------------------------------------------------------------------
// Runner
// ---------------------------------------------------------------------------
int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_status_ok_mid_range);
    RUN_TEST(test_status_dry_below_threshold);
    RUN_TEST(test_status_critical_below_threshold);
    RUN_TEST(test_status_wet_above_threshold);
    RUN_TEST(test_percent_at_zero_adc);
    RUN_TEST(test_percent_at_max_adc);
    RUN_TEST(test_percent_clamped_never_exceeds_100);
    RUN_TEST(test_single_spike_does_not_flip_to_critical);
    return UNITY_END();
}
