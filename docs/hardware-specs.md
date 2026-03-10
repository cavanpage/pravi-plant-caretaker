# Hardware Interface Specification

## 1. Physical Layer: RJ45 Connectivity
The system utilizes standard Cat5e/Cat6 cabling and RJ45 connectors. This choice leverages twisted-pair geometry to reduce electromagnetic interference (EMI) on analog signal lines.

### Standardized Pinout (The Pravi Port)
| Pin | Function | Voltage | Description |
|:---|:---|:---|:---|
| 1 | VCC_LOGIC | 3.3V | Power for digital/analog sensors |
| 2 | VCC_ACTUATOR | 12V | High-power rail for Pumps/Solenoids |
| 3 | GND | 0V | Common ground reference |
| 4 | I2C_SDA | 3.3V | I2C Data line for digital sensors |
| 5 | I2C_SCL | 3.3V | I2C Clock line for digital sensors |
| 6 | SIG_A | 0-3.3V | Primary Analog Input / Interrupt Pin |
| 7 | SIG_B | 0-3.3V | Secondary Analog Input / Interrupt Pin |
| 8 | MOD_ID | 0-3.3V | Resistor-based Module Identification |

---

## 2. Electrical Protection
The Atmel SAM3X8E (Arduino Due) operates strictly at 3.3V logic. 

### Actuator Control
To drive 12V loads (Pumps/Solenoids), the system requires an opto-isolated relay or MOSFET driver. All inductive loads must include a 1N4007 flyback diode across the terminals.

### 5V Sensor Integration (Voltage Dividers)
When using 5V sensors, a voltage divider is required to scale the output to 3.3V.


---

## 3. Module Identification (MOD_ID)
Each modular add-on includes a specific resistor value connected between Pin 8 and Pin 3. Upon boot, the Due performs an analog read on this pin to identify the connected hardware.

| Resistor Value | Identified Module |
|:---|:---|
| 1.0 kΩ | Base Capacitive Moisture Sensor |
| 4.7 kΩ | Reservoir Kit (Pump + Level Sensor) |
| 10.0 kΩ | Direct-Line Kit (Solenoid + Flow Meter) |
