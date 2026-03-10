# Software Architecture & Interlocks

## Volumetric Flow Control
For professional setups, Pravi uses interrupt-driven pulse counting to calculate "Gallons per Minute" (GPM) or Liters, ensuring accurate delivery regardless of water pressure fluctuations.


```cpp
// ISR for Hall-Effect Flow Sensor
void pulseCounter() {
    pulseCount++;
}

void executeWaterCycle(float targetLiters) {
    if (!safetyInterlockActive()) return;
    
    openSolenoid();
    unsigned long startTime = millis();
    
    while (currentVolume < targetLiters && (millis() - startTime < WATCHDOG_TIMEOUT)) {
        currentVolume = (pulseCount / calibrationFactor);
        
        // Safety: If valve is open but no flow is detected, abort
        if (isStalled()) handleStallError();
    }
    closeSolenoid();
}
