## For ESP32 Camera Module 

### Connections For USB to TTL 
| Color    | Meaning  | ESP32-CAM |
| -------- | -------- | ------------------------ |
| 🔴 Red   | VCC (5V) | 5V pin                   |
| ⚫ Black  | GND      | GND                      |
| ⚪ White  | RX       | TX (U0T)                 |
| 🟢 Green | TX       | RX (U0R)                 |


### Connections (Specific for Flashing)
|ESP32-CAM Pin | ESP32-CAM Pin |
|--------------|---------------|
|IOO (GPIOO)   | GND |


### Select Board Type : AI Thinker ESP32CAM

# Workflow:

ESP32-CAM → (WiFi) → MQTT Broker → Laptop (YOLO) → Result


# 
```bash
uv install --lock uv.lock
```

# 

|ESP32-CAM |Arduino Uno| 
|----------|-----------|
|U0T|RX (Pin 0)|
|U0R|TX (Pin 1)|
|GND|GND|