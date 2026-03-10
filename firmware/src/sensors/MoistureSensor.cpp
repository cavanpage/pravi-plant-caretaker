#include "MoistureSensor.h"
#include <Arduino.h>

MoistureSensor::MoistureSensor()
    : _filter(), _filtered(0.0f), _status(MoistureStatus::OK)
{}

void MoistureSensor::update() {
    int raw = analogRead(PIN_SIG_A);
    _filtered = _filter.update(raw);
    _status   = classify(_filtered);
}

float MoistureSensor::rawFiltered() const {
    return _filtered;
}

float MoistureSensor::percent() const {
    // Map filtered ADC count to 0–100%.
    // Higher ADC count = wetter (capacitive sensor convention).
    float pct = (_filtered / 4095.0f) * 100.0f;
    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}

MoistureStatus MoistureSensor::status() const {
    return _status;
}

MoistureStatus MoistureSensor::classify(float value) const {
    if (value < MOISTURE_CRITICAL) return MoistureStatus::CRITICAL;
    if (value < MOISTURE_DRY)      return MoistureStatus::DRY;
    if (value > MOISTURE_WET)      return MoistureStatus::WET;
    return MoistureStatus::OK;
}
