# 🚒 Multifunctional Fireguard-Bot (FireBot Pro)

![Project Status](https://img.shields.io/badge/Status-Active-success)
![Hardware](https://img.shields.io/badge/Hardware-Arduino_Uno_%7C_ESP32--CAM-blue)
![Software](https://img.shields.io/badge/Software-C++_%7C_Python_%7C_YOLOv11-yellow)
![License](https://img.shields.io/badge/License-MIT-green)

An autonomous and manually controllable multifunctional rover featuring a **Primary-Secondary** communication architecture. The robot utilizes an **ESP32-CAM** (Primary Controller) for Wi-Fi communication, Web UI hosting, and camera streaming, paired with an **Arduino Uno** (Secondary Node) for real-time sensor processing and fire extinguishing mechanics. It supports Line Following, autonomous Firefighting, and AI-driven navigation using **YOLOv11**.

## 📑 Table of Contents
- [Key Features](#-key-features)
- [System Architecture](#-system-architecture)
- [Hardware Requirements](#-hardware-requirements)
- [Wiring & Connections](#-wiring--connections)
- [Installation & Setup](#-installation--setup)
- [Usage & Modes](#-usage--modes)
- [YOLO AI Setup](#-yolo-ai-setup)
- [Author](#-author)

---

## ✨ Key Features
* 🎮 **Manual Control (Web UI):** Drive the bot via a responsive mobile/desktop web interface with adjustable PWM speed control (50-255).
* 🛤️ **Line Follower Mode:** Autonomous navigation following designated paths using dual IR sensors.
* 🔥 **Firefighter Mode:** Triple flame-sensor array detects fire position (Left, Center, Right). Automatically stops, triggers an active buzzer, and uses a servo-mounted water cannon to sweep and extinguish the fire.
* 🧠 **YOLOv11 AI Navigation:** Real-time object detection via laptop processing. The bot streams video to the laptop, which runs YOLOv11 to detect obstacles and sends HTTP GET commands back to the ESP32 to dodge them.
* 🛑 **Global Emergency Override:** A zero-latency emergency stop button in the Web UI that instantly locks all motors, pumps, and sensors, displaying a full-screen lockdown overlay.

---

## 🏗️ System Architecture
The project uses a **Primary-Secondary Serial Communication** model (`9600 baud`):
1. **ESP32-CAM (Primary Controller):** Hosts the SoftAP Wi-Fi (`SSID: FireBot`), serves the HTML/JS Web UI, controls the L298N Motor Driver via PWM, and streams JPEG frames. 
2. **Arduino Uno (Secondary Node):** Polls hardware sensors (IR, Flame) at high frequencies and controls the water pump, servo, and buzzer. It sends movement requests (Forward, Left, Right, Stop) to the ESP32 based on sensor data.

---

## 🛠️ Hardware Requirements
* **Microcontrollers:** ESP32-CAM (with FTDI programmer), Arduino Uno R3.
* **Actuators:** 4x DC Gear Motors, L298N Motor Driver, 5V Mini Water Pump, SG90 Micro Servo.
* **Sensors:** 2x IR Tracking Sensors, 3x Flame Sensors.
* **Misc:** Active Buzzer, Power Supply (e.g., 2x 18650 Li-ion batteries), Jumper Wires, Chassis.

---

## 🔌 Wiring & Connections

### ESP32-CAM to Motor Driver (L298N)
| ESP32-CAM Pin | L298N Pin | Description |
| :--- | :--- | :--- |
| GPIO 14 | IN1 | Left Motor Forward |
| GPIO 15 | IN2 | Left Motor Backward |
| GPIO 13 | IN3 | Right Motor Forward |
| GPIO 12 | IN4 | Right Motor Backward |

### Arduino Uno to Sensors & Actuators
| Arduino Pin | Component | Description |
| :--- | :--- | :--- |
| D4 | Left IR Sensor | Line tracking |
| D5 | Right IR Sensor | Line tracking |
| D6 | Flame Sensor (Left) | Fire detection |
| D7 | Flame Sensor (Front)| Fire detection |
| D8 | Flame Sensor (Right)| Fire detection |
| D9 | Servo Motor | Water cannon sweeper |
| D12 | Water Pump (Relay) | Sprays water (Active LOW) |
| D3 | Active Buzzer | Fire alert alarm |

### Inter-Microcontroller Communication
* **Arduino Pin 10 (RX)** ➡️ **ESP32-CAM TX**
* **Arduino Pin 11 (TX)** ➡️ **ESP32-CAM RX**
* *(Note: Ensure common Ground between Arduino, ESP32, and power supplies).*

---

## 🚀 Installation & Setup

### 1. Arduino Setup
1. Open the Arduino IDE.
2. Install the standard `<Servo.h>` and `<SoftwareSerial.h>` libraries.
3. Upload `arduino_code.ino` to the Arduino Uno.

### 2. ESP32-CAM Setup
1. Install the ESP32 Board Package in Arduino IDE.
2. Select **AI Thinker ESP32-CAM** from the boards menu.
3. Upload `esp32_cam_code.ino` using an FTDI module.
4. *Troubleshooting:* If you face camera init errors, ensure PSRAM is enabled and the FTDI provides sufficient 5V power.

---

## 🕹️ Usage & Modes
1. Power on the robot.
2. Connect your smartphone or laptop to the Wi-Fi network: **SSID: `FireBot`** (No password).
3. Open a web browser and go to `http://192.168.4.1`.
4. Select your desired mode from the top tabs:
   * **MANUAL:** Use the D-Pad to move and buttons to adjust speed.
   * **LINE:** The bot will start following black lines.
   * **FIRE:** The bot will roam/standby until it detects a flame, then extinguish it.
   * **YOLO AI:** Engages the endpoint for Python-based remote control.

---

## 👁️ YOLO AI Setup (Laptop/PC)
To run the autonomous obstacle avoidance script:

1. Ensure your laptop is connected to the `FireBot` Wi-Fi network.
2. Install Python dependencies:
   ```bash
   pip install opencv-python ultralytics requests
   ```
3. Run the inference script:
   ```bash
   python yolo_navigation.py
   ```
4. The script will fetch the stream from `http://192.168.4.1/stream`, process bounding boxes, and automatically send HTTP GET commands (e.g., `/action?cmd=L`) to steer the bot away from obstacles.

