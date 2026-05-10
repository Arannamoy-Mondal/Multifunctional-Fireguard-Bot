# 🔥 Multifunctional Fireguard Bot

An ESP32-based intelligent robotic system designed for early fire detection, autonomous navigation, and remote response. This project integrates line following, obstacle avoidance via AI vision, real-time flame detection, and a Wi-Fi-based smartphone control interface into a single, cohesive platform.

## 📑 Table of Contents
- [Project Overview](#project-overview)
- [Key Features & Modes](#key-features--modes)
- [Hardware Architecture](#hardware-architecture)
- [Pin Configuration](#pin-configuration)
- [Software & Tech Stack](#software--tech-stack)
- [Installation & Setup](#installation--setup)
- [How to Operate](#how-to-operate)
- [Future Enhancements](#future-enhancements)
- [Credits & Team](#credits--team)

---

## 🚀 Project Overview
Traditional fire detection systems rely on fixed sensors, which can delay response times. The Multifunctional Fireguard Bot solves this by acting as a mobile "intelligent sentinel." It operates in both autonomous and manual modes, serving as a low-cost robotic solution to enhance safety in homes, industries, and smart buildings. 

The system uses a **Primary-Secondary communication model** where an Arduino Uno handles low-level sensor reading and actuation, while an ESP32-CAM manages Wi-Fi, camera streaming, and high-level motor control.

---

## ⚙️ Key Features & Modes

The robot features four distinct operational modes, accessible via a custom Web UI:

1. **🎮 Manual Mode:** Full control of the robot using a virtual D-Pad on the web dashboard. Users can adjust motor speed using PWM (50-255).
2. **〰️ Line Follower Mode:** Utilizes dual IR sensors to autonomously detect and follow predefined black paths. The Arduino calculates movements (Forward, Left, Right) and sends them to the ESP32.
3. **🔥 Firefighter Mode:** Acts as a radar, utilizing three flame sensors (Left, Front, Right). Upon detecting a fire hazard, the robot instantly stops, locks its state, and activates a loud buzzer alert.
4. **👁️ YOLO AI Mode (Obstacle Avoidance):** Streams real-time MJPEG video to a connected laptop. A Python script running **YOLOv11** detects obstacles in the path and dynamically sends HTTP commands back to the ESP32 to autonomously steer clear of them.
5. **🛑 Emergency Stop:** A global fail-safe button that immediately halts all motors and operations across all modes.

---

## 🛠️ Hardware Architecture

| Component | Role in System | Qty |
| :--- | :--- | :--- |
| **ESP32-CAM** | Primary controller, Wi-Fi Server, Camera streaming | 1 |
| **Arduino Uno** | Secondary controller, handles sensors & alarms | 1 |
| **L298N Motor Driver** | Controls the locomotion of the robot | 1 |
| **DC Gear Motors** | Wheel actuation | 4 |
| **Flame Sensors** | Detects fire presence and direction | 3 |
| **IR Tracking Sensors**| Reads black lines for path following | 2 |
| **Active Buzzer** | Provides audible fire alerts | 1 |
| **18650 Li-ion** | Main power supply | 3 |

---

## 🔌 Pin Configuration

### 1. ESP32-CAM Wiring (Primary Controller)
**Motor Driver (L298N):**
* `IN1` (Left Motor Forward) ➔ **GPIO 14**
* `IN2` (Left Motor Backward) ➔ **GPIO 15**
* `IN3` (Right Motor Forward) ➔ **GPIO 13**
* `IN4` (Right Motor Backward) ➔ **GPIO 12**

**Camera Module (Ai-Thinker Default):**
* `PWDN`: 32 | `RESET`: -1 | `XCLK`: 0 | `SIOD`: 26 | `SIOC`: 27 | `VSYNC`: 25 | `HREF`: 23 | `PCLK`: 22
* `Data Pins`: Y9(35), Y8(34), Y7(39), Y6(36), Y5(21), Y4(19), Y3(18), Y2(5)

### 2. Arduino Uno Wiring (Secondary Controller)
**Sensors & Actuators:**
* `Left IR Sensor` ➔ **Pin 4**
* `Right IR Sensor` ➔ **Pin 5**
* `Left Flame Sensor` ➔ **Pin 6**
* `Front Flame Sensor` ➔ **Pin 7**
* `Right Flame Sensor` ➔ **Pin 8**
* `Active Buzzer` ➔ **Pin 3**

**ESP32-CAM to Arduino Serial Communication:**
* `Arduino RX (Pin 10)` ➔ **ESP32 TX**
* `Arduino TX (Pin 11)` ➔ **ESP32 RX**

---

## 💻 Software & Tech Stack
* **Microcontrollers:** C++ (Arduino IDE) with `esp_camera.h` and `SoftwareSerial`.
* **Web UI:** HTML, CSS, JavaScript (Hosted directly on the ESP32's memory).
* **AI & Vision:** Python, OpenCV, Ultralytics YOLOv11 (`yolo11n.pt`).

---

## 🔧 Installation & Setup

### 1. Arduino Uno Setup
1. Open the Arduino IDE.
2. Upload the `arduino_code.ino` file to the Arduino Uno.
3. Ensure the Arduino is connected to the ESP32 via SoftwareSerial (Pins 10 & 11).

### 2. ESP32-CAM Setup
1. Install the ESP32 board manager in the Arduino IDE.
2. Select the `AI Thinker ESP32-CAM` board.
3. Upload the `esp32_cam_code.ino` file. *(Note: Make sure your FTDI programmer is set to 5V and GPIO0 is grounded during upload).*

### 3. Python AI Setup (Optional: For YOLO Mode)
1. Install Python 3.8+ on your laptop.
2. Install the required libraries:
   ```bash
   pip install opencv-python requests numpy ultralytics

```

3. Ensure the laptop is connected to the robot's Wi-Fi network.

---

## 🕹️ How to Operate

1. **Power On:** Turn on the power supply. The ESP32-CAM will initialize and create a Wi-Fi SoftAP.
2. **Connect to Wi-Fi:** Connect your smartphone or laptop to the Wi-Fi network named **`FireBot`**.
3. **Access Dashboard:** Open a web browser and navigate to `http://192.168.4.1`.
4. **Select Mode:** Use the top tabs (MANUAL, LINE, FIRE, YOLO AI) to switch between operational modes.
5. **Run AI Vision:** If testing the YOLO AI mode, run the Python script on your laptop:
```bash
python yolo_navigation.py

```





