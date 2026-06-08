# Auto-Cooler: Closed-Loop Temperature Control System (STM32F4)

##  Overview

Auto-Cooler is a real-time embedded temperature control system implemented on the **STM32F4 microcontroller** and validated using **Proteus simulation**.

The system continuously monitors ambient temperature using an **LM35 sensor** and automatically controls a DC fan speed via **PWM** to maintain a stable environment.

It is designed using a **Mealy Finite State Machine (FSM)** to ensure deterministic and structured system behavior with real-time feedback displayed on an **LCD1602**.

<img width="1139" height="852" alt="image" src="https://github.com/user-attachments/assets/01ecb48a-f0d9-436b-8465-4891e753ba2d" />

---

## 🧪 Simulation Environment

* 🖥️ **Proteus Design Suite**
*  STM32F4 Microcontroller Simulation
*  LM35 Temperature Sensor Simulation
*  Fan Model
*  LCD1602 Virtual Display
*  LED Indicators

---

## ✨ Features

*  Real-time temperature monitoring using LM35
*  ADC in continuous conversion mode (12-bit resolution)
*  DMA-based data acquisition (efficient, non-blocking)
*  PWM-based fan speed control using STM32 timers
*  LCD1602 live display (temperature + fan duty cycle)
*  Mealy Finite State Machine (IDLE / COOLING / OVERHEAT)
*  Overheat protection at 40°C
*  Alarm LED activation in critical conditions
*  Fully modular driver-based architecture

---

## 🌡️ Control Logic

### 🌬️ Fan Speed Mapping

| Temperature | Fan Duty Cycle |
| ----------- | -------------- |
| < 25°C      | 0% (OFF)       |
| 25°C – 30°C | 33%            |
| 30°C – 35°C | 66%            |
| ≥ 35°C      | 100%           |

---

## 🔥 Overheat Protection

When temperature reaches **40°C or higher**:

*  Fan runs at full speed (100%)
*  LCD displays OVERHEAT warning
*  Alarm LED is activated
*  System transitions to OVERHEAT state

---

## 🔄 Finite State Machine (FSM)

### 💤 IDLE State

* Fan OFF
* Temperature continuously monitored
* Transition to COOLING when T ≥ 25°C

---

### ❄️ COOLING State

* Fan speed dynamically adjusted based on temperature
* LCD displays real-time values
* Transitions:

  * 🔼 OVERHEAT if T ≥ 40°C
  * 🔽 IDLE if T < 25°C

---

### 🔥 OVERHEAT State

* Fan locked at 100%
* Alarm LED ON
* LCD shows warning message
* Returns to COOLING when temperature drops below 40°C

