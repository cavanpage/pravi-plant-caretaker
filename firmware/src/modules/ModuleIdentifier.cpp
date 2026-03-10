#include "ModuleIdentifier.h"
#include <Arduino.h>

uint8_t ModuleIdentifier::_detected = MODID_NONE;

uint8_t ModuleIdentifier::detect() {
    int raw = analogRead(PIN_MOD_ID);

    if (raw >= MODID_MOISTURE_LOW && raw <= MODID_MOISTURE_HIGH) {
        _detected = MODID_MOISTURE;
    } else if (raw >= MODID_RESERVOIR_LOW && raw <= MODID_RESERVOIR_HIGH) {
        _detected = MODID_RESERVOIR;
    } else if (raw >= MODID_DIRECTLINE_LOW && raw <= MODID_DIRECTLINE_HIGH) {
        _detected = MODID_DIRECT_LINE;
    } else {
        _detected = MODID_NONE;
    }

    Serial.print("[ModuleIdentifier] MOD_ID ADC=");
    Serial.print(raw);
    Serial.print(" → ");
    Serial.println(label(_detected));

    return _detected;
}

uint8_t ModuleIdentifier::current() {
    return _detected;
}

const char* ModuleIdentifier::label(uint8_t modId) {
    switch (modId) {
        case MODID_MOISTURE:    return "Base Moisture Sensor";
        case MODID_RESERVOIR:   return "Reservoir Kit";
        case MODID_DIRECT_LINE: return "Direct-Line Kit";
        default:                return "Unknown / None";
    }
}
