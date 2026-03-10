#pragma once
// ---------------------------------------------------------------------------
// Arduino HAL mock for native (host) unit testing via PlatformIO + Unity.
//
// Hardware state is stored in a function-local static (singleton), so ALL
// translation units share the same backing store — no per-TU copies.
//
// Tests control hardware state via the mock_* macros below.
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
#define IRAM_ATTR

// Analog pin aliases (small integers so they fit the backing arrays)
#define A0  0
#define A1  1
#define A2  2
#define A3  3
#define A4  4
#define A5  5

// ---------------------------------------------------------------------------
// Shared mock state — one instance across all TUs via inline function-local static
// ---------------------------------------------------------------------------
namespace mock_arduino {
    struct State {
        int           analog_values[16];
        int           digital_values[32];
        unsigned long millis_value;

        State() : millis_value(0) {
            memset(analog_values,  0, sizeof(analog_values));
            memset(digital_values, 0, sizeof(digital_values));
        }
    };

    inline State& state() {
        static State s;
        return s;
    }

    inline void reset() {
        state() = State();
    }
}

// Convenience macros — use these in tests to drive hardware state
#define mock_analog_values  (mock_arduino::state().analog_values)
#define mock_digital_values (mock_arduino::state().digital_values)
#define mock_millis_value   (mock_arduino::state().millis_value)
#define mock_reset()        (mock_arduino::reset())

// ---------------------------------------------------------------------------
// HAL stubs
// ---------------------------------------------------------------------------
inline int           analogRead(int pin)            { return mock_arduino::state().analog_values[pin % 16]; }
inline void          digitalWrite(int pin, int val) { mock_arduino::state().digital_values[pin % 32] = val; }
inline int           digitalRead(int pin)           { return mock_arduino::state().digital_values[pin % 32]; }
inline void          pinMode(int, int)              {}
inline unsigned long millis()                       { return mock_arduino::state().millis_value; }
inline void          delay(unsigned long)           {}
inline void          analogReadResolution(int)      {}
inline int           digitalPinToInterrupt(int pin) { return pin; }
inline void          attachInterrupt(int, void(*)(), int) {}

// ---------------------------------------------------------------------------
// Minimal Serial mock
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

inline SerialMock& Serial_instance() {
    static SerialMock s;
    return s;
}

#define Serial (Serial_instance())
