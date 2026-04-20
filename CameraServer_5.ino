#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// --- WiFi Access Point Settings ---
const char* ssid = "FireBot";         
const char* password = ""; 

WebServer server(80);

// --- Motor Pins (ESP32-CAM GPIO) ---
#define IN1 14 
#define IN2 15 
#define IN3 13 
#define IN4 12 

// --- Global Variables ---
int robotSpeed = 200; 
char currentMode = 'M'; // ডিফল্টভাবে ম্যানুয়াল মোড

// অবস্টাকল ওভাররাইডের জন্য
unsigned long obstacleTimer = 0;
bool obstacleActive = false;

// --- Ai-Thinker ESP32-CAM Pin Configuration ---
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// --- Text-Only Tabbed Mobile UI ---
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>FireBot Interface</title>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background-color: #2c3e50; color: white; margin: 0; padding: 10px; }
    h2 { margin-top: 5px; font-size: 20px; }
    img { width: 100%; max-width: 640px; border: 4px solid #34495e; border-radius: 10px; margin-bottom: 15px; }
    
    /* Tab Styling */
    .tabs { display: flex; justify-content: center; gap: 10px; max-width: 340px; margin: 0 auto 15px auto; }
    .tab-btn { flex: 1; padding: 12px; font-size: 14px; font-weight: bold; border: none; border-radius: 5px; cursor: pointer; color: white; background-color: #7f8c8d; transition: 0.3s; }
    
    /* Active sections */
    .control-section { display: none; }
    .active-section { display: block; }
    
    /* D-Pad Styling */
    .pad { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; max-width: 340px; margin: 0 auto; }
    .btn { background-color: #3498db; color: white; border: none; padding: 15px 5px; font-size: 14px; font-weight: bold; border-radius: 10px; cursor: pointer; user-select: none; }
    .btn:active { background-color: #2980b9; }
    .btn-red { background-color: #e74c3c; padding: 20px 5px; }
    .btn-red:active { background-color: #c0392b; }
    .empty { visibility: hidden; }
    
    /* Speed Control Styling */
    .speed-control { margin-top: 20px; display: flex; justify-content: center; align-items: center; gap: 15px; }
    .speed-text { font-size: 16px; font-weight: bold; }
    .btn-speed { background-color: #f39c12; color: white; border: none; padding: 10px 15px; font-size: 12px; font-weight: bold; border-radius: 5px; cursor: pointer; }
    
    /* Auto Mode Status Box */
    .auto-status { padding: 30px 10px; background-color: #34495e; border-radius: 10px; max-width: 320px; margin: 0 auto; }
    .status-text { font-size: 18px; font-weight: bold; color: #2ecc71; margin-bottom: 20px; }
  </style>
</head>
<body onload="initTab()">
  <h2>SYSTEM CONTROL</h2>
  <img id="camFeed" src="/capture" alt="Camera Feed">
  
  <div class="tabs">
    <button id="tabManual" class="tab-btn" onclick="switchTab('M')">MANUAL CONTROL</button>
    <button id="tabAuto" class="tab-btn" onclick="switchTab('A')">LINE FOLLOWER</button>
  </div>
  
  <div id="manualControls" class="control-section active-section">
    <div class="pad">
      <div class="empty"></div>
      <button class="btn" onmousedown="sendCmd('F')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('F')" ontouchend="sendCmd('S')">FORWARD</button>
      <div class="empty"></div>
      <button class="btn" onmousedown="sendCmd('L')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('L')" ontouchend="sendCmd('S')">LEFT</button>
      <button class="btn btn-red" onmousedown="sendCmd('S')" ontouchstart="sendCmd('S')">STOP</button>
      <button class="btn" onmousedown="sendCmd('R')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('R')" ontouchend="sendCmd('S')">RIGHT</button>
      <div class="empty"></div>
      <button class="btn" onmousedown="sendCmd('B')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('B')" ontouchend="sendCmd('S')">BACKWARD</button>
      <div class="empty"></div>
    </div>
    <div class="speed-control">
      <button class="btn-speed" onclick="changeSpeed(-25)">SPEED DOWN</button>
      <div class="speed-text">SPEED: <span id="speedDisplay">200</span></div>
      <button class="btn-speed" onclick="changeSpeed(25)">SPEED UP</button>
    </div>
  </div>

  <div id="autoControls" class="control-section">
    <div class="auto-status">
      <div class="status-text">SYSTEM IS FOLLOWING LINE</div>
      <button class="btn btn-red" style="width: 100%; font-size: 16px;" onclick="sendCmd('S')">EMERGENCY STOP</button>
    </div>
  </div>

  <script>
    let currentSpeed = 200;

    // Set initial Tab colors
    function initTab() {
      document.getElementById('tabManual').style.backgroundColor = '#e67e22'; // Orange for manual
    }

    // Handle switching between the two interfaces
    function switchTab(mode) {
      if(mode === 'A') {
        document.getElementById('manualControls').classList.remove('active-section');
        document.getElementById('autoControls').classList.add('active-section');
        document.getElementById('tabAuto').style.backgroundColor = '#27ae60'; // Green
        document.getElementById('tabManual').style.backgroundColor = '#7f8c8d'; // Gray
      } else {
        document.getElementById('autoControls').classList.remove('active-section');
        document.getElementById('manualControls').classList.add('active-section');
        document.getElementById('tabManual').style.backgroundColor = '#e67e22'; // Orange
        document.getElementById('tabAuto').style.backgroundColor = '#7f8c8d'; // Gray
      }
      fetch('/mode?val=' + mode).catch(e => console.log(e));
    }

    function sendCmd(cmd) { fetch('/action?cmd=' + cmd).catch(e => console.log(e)); }
    
    function changeSpeed(step) {
      currentSpeed += step;
      if (currentSpeed > 255) currentSpeed = 255;
      if (currentSpeed < 100) currentSpeed = 100;
      document.getElementById('speedDisplay').innerText = currentSpeed;
      fetch('/speed?val=' + currentSpeed).catch(e => console.log(e));
    }
    
    // Live video refresh
    setInterval(function() { 
      document.getElementById('camFeed').src = '/capture?r=' + Math.random(); 
    }, 150); 
  </script>
</body>
</html>
)rawliteral";

// --- PWM Motor Control Functions (ESP32 Core 3.x.x) ---
void setupMotors() {
  ledcAttach(IN1, 2000, 8);
  ledcAttach(IN2, 2000, 8);
  ledcAttach(IN3, 2000, 8);
  ledcAttach(IN4, 2000, 8);
  stopMotors();
}

void moveForward() { ledcWrite(IN1, robotSpeed); ledcWrite(IN2, 0); ledcWrite(IN3, robotSpeed); ledcWrite(IN4, 0); }
void moveBackward() { ledcWrite(IN1, 0); ledcWrite(IN2, robotSpeed); ledcWrite(IN3, 0); ledcWrite(IN4, robotSpeed); }
void turnRight() { ledcWrite(IN1, robotSpeed); ledcWrite(IN2, 0); ledcWrite(IN3, 0); ledcWrite(IN4, robotSpeed); }
void turnLeft() { ledcWrite(IN1, 0); ledcWrite(IN2, robotSpeed); ledcWrite(IN3, robotSpeed); ledcWrite(IN4, 0); }
void stopMotors() { ledcWrite(IN1, 0); ledcWrite(IN2, 0); ledcWrite(IN3, 0); ledcWrite(IN4, 0); }

// --- Camera Setup ---
void setup_camera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0; config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM; config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM; config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM; config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000; config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA; config.jpeg_quality = 12; config.fb_count = 1;
  
  if(esp_camera_init(&config) != ESP_OK) { Serial.println("Camera Init Failed"); }
}

// --- Web Endpoints ---
void handleRoot() { server.send(200, "text/html", INDEX_HTML); }

void handleCapture() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) { server.send(500, "text/plain", "Camera error"); return; }
  server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

void handleCommand() {
  if (server.hasArg("cmd")) {
    char cmd = server.arg("cmd")[0];
    
    // ল্যাপটপ থেকে অবস্টাকল সিগন্যাল
    if (cmd == 'O') {
      obstacleActive = true;
      obstacleTimer = millis(); 
      stopMotors();             
      server.send(200, "text/plain", "Obstacle Override");
      return;
    }

    // ম্যানুয়াল মোডে ডিরেকশন কমান্ড এবং ইমারজেন্সি স্টপ
    if (currentMode == 'M' || cmd == 'S') {
      if (cmd == 'F') moveForward(); 
      else if (cmd == 'B') moveBackward();
      else if (cmd == 'L') turnLeft(); 
      else if (cmd == 'R') turnRight();
      else if (cmd == 'S') stopMotors();
      server.send(200, "text/plain", "OK");
    } else { 
      server.send(400, "text/plain", "Ignored: In Auto Mode"); 
    }
  }
}

void handleSpeed() {
  if (server.hasArg("val")) { robotSpeed = server.arg("val").toInt(); server.send(200, "text/plain", "Speed Updated"); }
}

void handleMode() {
  if (server.hasArg("val")) {
    currentMode = server.arg("val")[0];
    stopMotors(); 
    obstacleActive = false; 
    server.send(200, "text/plain", "Mode Updated");
  }
}

void setup() {
  Serial.begin(9600); 
  setupMotors();
  setup_camera();
  WiFi.softAP(ssid, password);
  server.on("/", HTTP_GET, handleRoot); 
  server.on("/capture", HTTP_GET, handleCapture);
  server.on("/action", HTTP_GET, handleCommand); 
  server.on("/speed", HTTP_GET, handleSpeed);
  server.on("/mode", HTTP_GET, handleMode);
  server.begin();
}

void loop() {
  server.handleClient();

  if (obstacleActive) {
    if (millis() - obstacleTimer >= 10000) { obstacleActive = false; }
  } else {
    if (currentMode == 'A' && Serial.available()) {
      char autoCmd = Serial.read();
      if (autoCmd == 'F') moveForward();
      else if (autoCmd == 'L') turnLeft();
      else if (autoCmd == 'R') turnRight();
      else if (autoCmd == 'S') stopMotors();
    }
  }
}