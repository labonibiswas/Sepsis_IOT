# 🩺 Sepsis IoT Monitoring System

An **IoT-based Sepsis Monitoring System** built using an **ESP32 microcontroller** to continuously monitor vital-sign parameters and identify combinations of abnormal readings that may indicate a potential sepsis condition.

The system provides **real-time visual and audible alerts** using an LCD display, LEDs, and a buzzer. The project also includes a **Wokwi simulation** for testing the system without physical hardware.

> **Important:** This project is an educational/prototype system and is **not intended for clinical diagnosis or medical decision-making**.

---

## 📌 Table of Contents

* [Overview](#-overview)
* [Features](#-features)
* [System Architecture](#-system-architecture)
* [Hardware Components](#-hardware-components)
* [Pin Configuration](#-pin-configuration)
* [Software Requirements](#-software-requirements)
* [Project Structure](#-project-structure)
* [Installation](#-installation)
* [Running the Simulation](#-running-the-simulation)
* [How It Works](#-how-it-works)
* [Sepsis Detection Logic](#-sepsis-detection-logic)
* [Output and Alerts](#-output-and-alerts)
* [Serial Monitor](#-serial-monitor)
* [Dependencies](#-dependencies)
* [Limitations](#-limitations)


---

## 🔎 Overview

Sepsis is a life-threatening medical condition in which the body's response to an infection can cause organ dysfunction. Early identification of abnormal physiological parameters can support timely clinical assessment.

This project demonstrates a prototype approach to monitoring several vital-sign parameters using an ESP32:

* 🌡️ Body temperature
* ❤️ Heart rate
* 🫁 Respiratory rate
* 🩸 Blood pressure
* 💧 Blood oxygen saturation (SpO₂)

The ESP32 evaluates the collected values against predefined threshold conditions. If multiple abnormal conditions are detected simultaneously, the system enters a **SEPSIS POSITIVE** alert state.

The repository is implemented as a PlatformIO project targeting the **ESP32 DevKit** and uses the Arduino framework.

---

## ✨ Features

* Real-time vital-sign monitoring
* ESP32-based embedded implementation
* DHT22 temperature sensor support
* Analog input simulation for physiological parameters
* 16×2 I2C LCD display
* Green LED for normal status
* Red LED for alert status
* Audible buzzer alert
* Serial Monitor logging
* Multi-parameter sepsis detection logic
* Wokwi-compatible hardware simulation
* PlatformIO project configuration

---

## 🏗️ System Architecture

```text
                   ┌─────────────────────┐
                   │      Vital Signs     │
                   │                     │
                   │ • Temperature       │
                   │ • Heart Rate        │
                   │ • SpO₂              │
                   │ • Systolic BP       │
                   │ • Diastolic BP      │
                   │ • Respiratory Rate  │
                   └──────────┬──────────┘
                              │
                              ▼
                   ┌─────────────────────┐
                   │        ESP32        │
                   │                     │
                   │ Sensor Acquisition  │
                   │        +            │
                   │ Threshold Analysis  │
                   └──────────┬──────────┘
                              │
                    ┌─────────┼─────────┐
                    │         │         │
                    ▼         ▼         ▼
                 ┌─────┐   ┌─────┐   ┌────────┐
                 │ LCD │   │ LED │   │ Buzzer │
                 └─────┘   └─────┘   └────────┘
                    │         │         │
                    └─────────┼─────────┘
                              ▼
                       Status / Alert
```

---

## 🔧 Hardware Components

The Wokwi circuit included in the repository contains the following components:

| Component     | Quantity | Purpose                            |
| ------------- | -------: | ---------------------------------- |
| ESP32 DevKit  |        1 | Main microcontroller               |
| DHT22         |        1 | Temperature measurement            |
| Potentiometer |        5 | Simulated analog vital-sign inputs |
| 16×2 I2C LCD  |        1 | Display system status              |
| Green LED     |        1 | Normal condition indicator         |
| Red LED       |        1 | Sepsis alert indicator             |
| Buzzer        |        1 | Audible alert                      |

The potentiometers are used in the simulation to represent analog vital-sign measurements.

---

## 📍 Pin Configuration

The pin assignments are defined in `src/main.cpp`.

| Parameter / Device | ESP32 GPIO |
| ------------------ | ---------: |
| DHT22 Data         |    GPIO 15 |
| Heart Rate         |    GPIO 36 |
| SpO₂               |    GPIO 39 |
| Systolic BP        |    GPIO 34 |
| Diastolic BP       |    GPIO 35 |
| Respiratory Rate   |    GPIO 32 |
| Green LED          |     GPIO 2 |
| Red LED            |     GPIO 4 |
| Buzzer             |     GPIO 5 |
| LCD SDA            |    GPIO 21 |
| LCD SCL            |    GPIO 22 |

The LCD uses I2C address `0x27` and is configured as a 16×2 display.

---

## 💻 Software Requirements

### Required

* [PlatformIO](https://platformio.org/)
* Visual Studio Code
* PlatformIO IDE extension
* ESP32 development board support

### Optional

* [Wokwi](https://wokwi.com/) for simulation
* ESP32 hardware
* DHT22 sensor
* LCD 16×2 I2C
* LEDs and buzzer
* Appropriate sensors for physiological measurements

---

## 📁 Project Structure

```text
Sepsis_IOT/
│
├── .vscode/
│
├── include/
│
├── lib/
│
├── src/
│   └── main.cpp
│
├── test/
│
├── .gitignore
├── diagram.json
├── platformio.ini
├── wokwi.toml
└── README.md
```

The repository currently follows the standard PlatformIO project structure and includes Wokwi configuration files.

---

## 🚀 Installation

### 1. Clone the repository

```bash
git clone https://github.com/labonibiswas/Sepsis_IOT.git
cd Sepsis_IOT
```

### 2. Open the project

Open the project folder in **Visual Studio Code** with PlatformIO installed.

### 3. Build the project

From the PlatformIO terminal:

```bash
pio run
```

### 4. Upload to ESP32

Connect the ESP32 board to your computer and run:

```bash
pio run --target upload
```

Alternatively, use the **Upload** button provided by PlatformIO in Visual Studio Code.

---

## 🧪 Running the Simulation

The repository includes a `diagram.json` file containing the Wokwi circuit configuration. The simulation uses an ESP32 DevKit, DHT22, potentiometers, LCD, LEDs, and buzzer.

You can open the project in Wokwi and use the potentiometers to simulate changes in the analog vital-sign inputs.

The Wokwi configuration is useful for:

* Testing the sepsis detection logic
* Simulating abnormal vital signs
* Testing LCD messages
* Testing LED behavior
* Testing the buzzer
* Debugging without physical hardware

---

## ⚙️ How It Works

The ESP32 initializes the sensors and output devices during startup.

```text
Start
  │
  ▼
Initialize ESP32
  │
  ├── Initialize DHT22
  ├── Initialize LCD
  ├── Configure LEDs
  └── Configure Buzzer
  │
  ▼
Read Vital Signs
  │
  ▼
Evaluate Thresholds
  │
  ▼
Count Abnormal Parameters
  │
  ├── AI RISK score > thresold or matched QSOFA criteria
  │       │
  │       ▼
  │   posibility of sepsis
  │       │
  │       ├── Red LED
  │       ├── Buzzer
  │       └── LCD Alert
  │
  └── Normal Vitals
          │
          ▼
        Healthy
          │
          ├── Green LED
          └── LCD Status
```

The implementation continuously reads the values and evaluates the predefined criteria.

---

## 🧠 Sepsis Detection Logic

The current implementation checks five categories of criteria:

| Parameter        | Current Condition                            |
| ---------------- | -------------------------------------------- |
| Blood Pressure   | SBP ≤ 100 **or** DBP ≤ 100                   |
| Respiratory Rate | RR ≥ 22                                      |
| Temperature      | Temperature > 38°C **or** Temperature < 36°C |
| SpO₂             | SpO₂ < 95%                                   |
| Heart Rate       | Heart rate > 90 BPM                          |

Each satisfied condition increments a `sepsisCriteriaCount`.

The current implementation considers the system **SEPSIS POSITIVE when two or more criteria are satisfied**.

### Simplified Logic

```text
criteria = 0

if abnormal blood pressure:
    criteria++

if respiratory rate >= 22:
    criteria++

if temperature > 38 or temperature < 36:
    criteria++

if SpO2 < 95:
    criteria++

if heart rate > 90:
    criteria++

if criteria >= 2:
    SEPSIS POSITIVE
else:
    SEPSIS NEGATIVE
```

When an abnormal condition is detected, its corresponding label is added to the alert message.

For example:

```text
Failed: BP RR HR
```

---

## 🚨 Output and Alerts

### 🟢 Normal State

When fewer than two criteria are abnormal:

The system:

* Turns the green LED ON
* Turns the red LED OFF
* Keeps the buzzer OFF
* Displays the normal status on the LCD

### 🔴 Alert State

When two or more criteria are abnormal:

The system:

* Turns the green LED OFF
* Flashes the red LED
* Activates the buzzer
* Displays the abnormal parameters on the LCD

The current implementation uses a 1000 Hz buzzer tone and repeats the alert sequence before continuing the monitoring cycle.

---

## 🖥️ Serial Monitor

The ESP32 communicates through the serial interface at:

```text
115200 baud
```

The program outputs the number of matched criteria and the abnormal parameters.

Example:

```text
Count: 0 | All vitals normal.
```

or:

```text
Count: 3 | Failed: RR SPO2 HR
```

This can be useful for debugging and observing the system's decision-making process.

---

## 📦 Dependencies

The project is configured using PlatformIO and the ESP32 Arduino framework.

Current dependencies:

```text
adafruit/DHT sensor library ^1.4.7
marcoschwartz/LiquidCrystal_I2C ^1.1.4
```

They are declared in `platformio.ini` and will be resolved by PlatformIO during the build process.

---

## 📊 Data Mapping in Simulation

The analog inputs are converted into approximate physiological ranges using ESP32 ADC values.

| Parameter        | ADC Range |   Simulated Range |
| ---------------- | --------: | ----------------: |
| Heart Rate       |    0–4095 |        40–180 BPM |
| SpO₂             |    0–4095 |           70–100% |
| Systolic BP      |    0–4095 |       80–180 mmHg |
| Diastolic BP     |    0–4095 |       40–110 mmHg |
| Respiratory Rate |    0–4095 | 10–40 breaths/min |

These values are generated using the Arduino `map()` function in the current implementation.

---

## ⚠️ Limitations

This project is intended as an **educational IoT prototype** rather than a medical-grade monitoring system.

### Current limitations include:

1. The heart rate, SpO₂, blood pressure, and respiratory-rate inputs are represented by analog values in the simulation.
2. The `map()` function converts ADC values into simulated physiological ranges rather than obtaining clinically validated measurements.
4. The system does not perform clinical diagnosis.
5. Sensor calibration and noise filtering are not currently implemented.
7. Patient identification and historical data management are not included.
8. The system has not been validated for clinical use.

**This system should not be used to make medical decisions or replace professional medical assessment.**

---


### IoT / Cloud

The ESP32 is extended with Wi-Fi connectivity to transmit patient data to a remote monitoring platform.

Architecture:

```text
Sensors
   │
   ▼
 ESP32
   │
   │ Wi-Fi
   ▼
Cloud / Server
   │
   ├── Database
   ├── Analytics
   └── Alert Service
          │
          ▼
      Dashboard


---
```
## 🧪 Example Test Scenarios

### Test 1 — Normal Patient

```text
Heart Rate:       75 BPM
SpO₂:             98%
SBP:             120 mmHg
DBP:              80 mmHg
Respiratory Rate: 16/min
Temperature:      37°C
```

Expected:

```text
Sepsis Negative
Green LED: ON
Red LED: OFF
Buzzer: OFF
```

### Test 2 — Multiple Abnormal Parameters

```text
Heart Rate:       110 BPM
SpO₂:              92%
Respiratory Rate: 25/min
```

Expected:

```text
SEPSIS POSITIVE
Failed: RR SPO2 HR
Red LED: FLASHING
Buzzer: ON
```

---

## 📚 Repository

Source code and simulation files are available on GitHub:

[Sepsis_IOT GitHub Repository](https://github.com/labonibiswas/Sepsis_IOT?utm_source=chatgpt.com)

---

## 🩺 Disclaimer

This project is a **research/educational prototype** demonstrating IoT-based physiological monitoring and rule-based alert generation.

It is **not a medical device**, has not been clinically validated, and must not be used as a substitute for diagnosis, treatment, or professional medical advice.
