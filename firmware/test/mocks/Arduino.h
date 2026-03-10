#pragma once
// ---------------------------------------------------------------------------
// Arduino HAL mock for native (host) unit testing via PlatformIO + Unity.
// Tests control hardware state by writing directly to mock_* variables.
// ---------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Arduino constants
// ---------------------------------------------------------------------------
#define HIGH         1
#define LOW          0
#define INPUT_PULLUP 0x02
#define OUTPUT       0x01
#define RISING       0x01
#define IRAM_ATTR           // no-op on native

// ---------------------------------------------------------------------------
// Controllable hardware state — write these from tests to drive behaviour
// ---------------------------------------------------------------------------
static int           mock_analog_values[16]  = {0};   // indexed by pin number
static int           mock_digital_values[16] = {0};   // indexed by pin number
static unsigned long mock_millis_value       = 0;

// ---------------------------------------------------------------------------
// HAL stubs
// ---------------------------------------------------------------------------
inline int           analogRead(int pin)              { return mock_analog_values[pin % 16]; }
inline void          digitalWrite(int pin, int val)   { mock_digital_values[pin % 16] = val; }
inline int           digitalRead(int pin)             { return mock_digital_values[pin % 16]; }
inline void          pinMode(int, int)                {}
inline unsigned long millis()                         { return mock_millis_value; }
inline void          delay(unsigned long)             {}
inline void          analogReadResolution(int)        {}
inline int           digitalPinToInterrupt(int pin)   { return pin; }
inline void          attachInterrupt(int, void(*)(), int) {}

// ---------------------------------------------------------------------------
// Minimal Serial mock — prints to stdout so test output is visible
// ---------------------------------------------------------------------------
struct SerialMock {
    void begin(int) {}
    operator bool() const { return true; }

    void print(const char* s)        { printf("%s", s); }
    void print(int v)                { printf("%d", v); }
    void print(float v, int dec = 2) { printf("%.*f", dec, v); }

    void println(const char* s)        { printf("%s\n", s); }
    void println(int v)                { printf("%d\n", v); }
    void println(float v, int dec = 2) { printf("%.*f\n", dec, v); }
    void println()                     { printf("\n"); }
};

static SerialMock Serial;

// ---------------------------------------------------------------------------
// Helper — reset all mock state between tests
// ---------------------------------------------------------------------------
inline void mock_reset() {
    memset(mock_analog_values,  0, sizeof(mock_analog_values));
    memset(mock_digital_values, 0, sizeof(mock_digital_values));
    mock_millis_value = 0;
}
