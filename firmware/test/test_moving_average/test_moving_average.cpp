#include <unity.h>
#include "filters/MovingAverage.h"

// ---------------------------------------------------------------------------
// setUp / tearDown — called by Unity before/after each test
// ---------------------------------------------------------------------------
void setUp()    {}
void tearDown() {}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

void test_initial_value_is_zero() {
    MovingAverage f;
    TEST_ASSERT_EQUAL_FLOAT(0.0f, f.value());
}

void test_single_sample_returns_that_value() {
    MovingAverage f;
    float result = f.update(2048);
    TEST_ASSERT_EQUAL_FLOAT(2048.0f, result);
}

void test_average_of_two_samples() {
    MovingAverage f;
    f.update(1000);
    float result = f.update(2000);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 1500.0f, result);
}

void test_window_fills_and_old_samples_drop() {
    MovingAverage f;
    // Fill window with 1000
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) f.update(1000);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 1000.0f, f.value());

    // Push FILTER_WINDOW_SIZE samples of 3000 — 1000s should all be evicted
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) f.update(3000);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 3000.0f, f.value());
}

void test_spike_rejection() {
    MovingAverage f;
    // Stable baseline
    for (int i = 0; i < FILTER_WINDOW_SIZE - 1; i++) f.update(2000);
    // One EMI spike
    f.update(4095);
    // Average should stay well below the spike
    TEST_ASSERT_LESS_THAN(2200.0f, f.value());
}

void test_reset_clears_state() {
    MovingAverage f;
    f.update(3000);
    f.update(3000);
    f.reset();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, f.value());
    // After reset, a single sample should return just that value
    TEST_ASSERT_EQUAL_FLOAT(500.0f, f.update(500));
}

void test_adc_max_does_not_overflow() {
    MovingAverage f;
    // All samples at 12-bit max (4095). Sum = 16 * 4095 = 65,520 — fits int32_t.
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) f.update(4095);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 4095.0f, f.value());
}

// ---------------------------------------------------------------------------
// Runner
// ---------------------------------------------------------------------------
int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_initial_value_is_zero);
    RUN_TEST(test_single_sample_returns_that_value);
    RUN_TEST(test_average_of_two_samples);
    RUN_TEST(test_window_fills_and_old_samples_drop);
    RUN_TEST(test_spike_rejection);
    RUN_TEST(test_reset_clears_state);
    RUN_TEST(test_adc_max_does_not_overflow);
    return UNITY_END();
}
