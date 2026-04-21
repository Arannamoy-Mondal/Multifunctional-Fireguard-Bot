#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "FireBot";
const char* password = "";

WebServer server(80);

// --- Motor Pins ---
#define IN1 14
#define IN2 15
#define IN3 13
#define IN4 12

// --- Global Variables ---
int robotSpeed = 255; 
char currentMode = 'M';
// --- New Code Start: Emergency State Variable ---
bool isEmergency = false; 
// --- New Code End ---

// --- Ai-Thinker ESP32-CAM Pins ---
#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM   0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27
#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM     5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

// --- HTML UI ---
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>FireBot Pro</title>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background-color: #2c3e50; color: white; margin: 0; padding: 10px; }
    img { width: 100%; max-width: 600px; border: 4px solid #34495e; border-radius: 10px; margin-bottom: 10px; }
    .tabs { display: flex; justify-content: center; gap: 5px; max-width: 340px; margin: 0 auto 15px auto; flex-wrap: wrap; }
    .tab-btn { flex: 1; padding: 10px 5px; font-size: 12px; font-weight: bold; border: none; border-radius: 5px; cursor: pointer; color: white; background-color: #7f8c8d; min-width: 70px; }
    .control-section { display: none; }
    .active-section { display: block; }
    .pad { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; max-width: 340px; margin: 0 auto; }
    .btn { background-color: #3498db; color: white; border: none; padding: 15px 5px; font-size: 14px; font-weight: bold; border-radius: 10px; cursor: pointer; }
    .btn-red { background-color: #e74c3c; }
    .speed-control { margin-top: 20px; display: flex; justify-content: center; align-items: center; gap: 15px; }
    .speed-text { font-size: 16px; font-weight: bold; }
    .btn-speed { background-color: #f39c12; color: white; border: none; padding: 10px 15px; font-size: 12px; font-weight: bold; border-radius: 5px; }
    .status-box { padding: 20px; background-color: #34495e; border-radius: 10px; max-width: 320px; margin: 0 auto; }
    
    /* --- New Code Start: Emergency Overlay CSS --- */
    #emergencyOverlay {
      display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%;
      background: rgba(192, 57, 43, 0.95); z-index: 9999; flex-direction: column;
      justify-content: center; align-items: center;
    }
    .resume-btn {
      background-color: #2ecc71; color: white; font-size: 24px; font-weight: bold;
      padding: 20px 40px; border: none; border-radius: 10px; cursor: pointer; margin-top: 20px;
    }
    /* --- New Code End --- */
  </style>
</head>
<body onload="switchTab('M')">

  <div id="emergencyOverlay">
    <h1 style="font-size: 40px; margin: 0;">EMERGENCY STOPPED!</h1>
    <p style="font-size: 18px;">All systems locked.</p>
    <button class="resume-btn" onclick="toggleEmergency(0)">RESUME SYSTEM</button>
  </div>
  <h2>FIREBOT SYSTEM</h2>
  <img id="camFeed" src="/capture" alt="Live Feed">
  
  <div class="tabs">
    <button id="tabManual" class="tab-btn" onclick="switchTab('M')">MANUAL</button>
    <button id="tabAuto" class="tab-btn" onclick="switchTab('A')">LINE</button>
    <button id="tabFire" class="tab-btn" onclick="switchTab('F')">FIRE</button>
    <button id="tabYolo" class="tab-btn" onclick="switchTab('Y')">YOLO AI</button>
  </div>
  
  <div id="manualControls" class="control-section">
    <div class="pad">
      <div></div>
      <button class="btn" onmousedown="sendCmd('F')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('F')" ontouchend="sendCmd('S')">FORWARD</button>
      <div></div>
      <button class="btn" onmousedown="sendCmd('L')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('L')" ontouchend="sendCmd('S')">LEFT</button>
      
      <button class="btn btn-red" onclick="toggleEmergency(1)">STOP</button>
      <button class="btn" onmousedown="sendCmd('R')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('R')" ontouchend="sendCmd('S')">RIGHT</button>
      <div></div>
      <button class="btn" onmousedown="sendCmd('B')" onmouseup="sendCmd('S')" ontouchstart="sendCmd('B')" ontouchend="sendCmd('S')">BACKWARD</button>
      <div></div>
    </div>
    <div class="speed-control">
      <button class="btn-speed" onclick="changeSpeed(-25)">SPEED DOWN</button>
      <div class="speed-text">SPEED: <span id="speedDisplay">255</span></div>
      <button class="btn-speed" onclick="changeSpeed(25)">SPEED UP</button>
    </div>
  </div>
  
  <div id="autoControls" class="control-section">
    <div class="status-box">
      <div style="color:#2ecc71;font-weight:bold;margin-bottom:15px;">AUTO MODE ACTIVE</div>
      <button class="btn btn-red" style="width:100%" onclick="toggleEmergency(1)">EMERGENCY STOP</button>
    </div>
  </div>

  <div id="fireControls" class="control-section">
    <div class="status-box">
      <div style="color:#e67e22;font-weight:bold;margin-bottom:15px;">FIREFIGHTER MODE ACTIVE</div>
      <button class="btn btn-red" style="width:100%" onclick="toggleEmergency(1)">EMERGENCY STOP</button>
    </div>
  </div>

  <div id="yoloControls" class="control-section">
    <div class="status-box">
      <div style="color:#9b59b6;font-weight:bold;margin-bottom:15px;">YOLO PYTHON CONTROL ACTIVE</div>
      <button class="btn btn-red" style="width:100%" onclick="toggleEmergency(1)">EMERGENCY STOP</button>
    </div>
  </div>

  <script>
    let currentSpeed = 255;
    
    function switchTab(mode) {
      document.querySelectorAll('.control-section').forEach(s => s.classList.remove('active-section'));
      document.querySelectorAll('.tab-btn').forEach(b => b.style.backgroundColor = '#7f8c8d');
      
      if (mode === 'M') {
        document.getElementById('manualControls').classList.add('active-section');
        document.getElementById('tabManual').style.backgroundColor = '#e67e22';
      } else if (mode === 'A') {
        document.getElementById('autoControls').classList.add('active-section');
        document.getElementById('tabAuto').style.backgroundColor = '#27ae60';
      } else if (mode === 'F') {
        document.getElementById('fireControls').classList.add('active-section');
        document.getElementById('tabFire').style.backgroundColor = '#c0392b';
      } else if (mode === 'Y') {
        document.getElementById('yoloControls').classList.add('active-section');
        document.getElementById('tabYolo').style.backgroundColor = '#8e44ad';
      }
      fetch('/mode?val=' + mode);
    }
    
    function sendCmd(cmd) { fetch('/action?cmd=' + cmd); }
    
    function changeSpeed(step) {
      currentSpeed = Math.min(255, Math.max(150, currentSpeed + step));
      document.getElementById('speedDisplay').innerText = currentSpeed;
      fetch('/speed?val=' + currentSpeed);
    }

    // --- New Code Start: Emergency Toggle Function ---
    function toggleEmergency(state) {
      if (state === 1) {
        document.getElementById('emergencyOverlay').style.display = 'flex';
        fetch('/emergency?state=1');
      } else {
        document.getElementById('emergencyOverlay').style.display = 'none';
        fetch('/emergency?state=0');
      }
    }
    // --- New Code End ---

    setInterval(function() {
      document.getElementById('camFeed').src = '/capture?r=' + Math.random();
    }, 150);
  </script>
</body>
</html>
)rawliteral";

// --- Motor Functions ---
void setupMotors() {
  ledcAttach(IN1, 2000, 8); ledcAttach(IN2, 2000, 8);
  ledcAttach(IN3, 2000, 8); ledcAttach(IN4, 2000, 8);
  stopMotors();
}
void moveForward()  { ledcWrite(IN1, robotSpeed); ledcWrite(IN2, 0); ledcWrite(IN3, robotSpeed); ledcWrite(IN4, 0); }
void moveBackward() { ledcWrite(IN1, 0); ledcWrite(IN2, robotSpeed); ledcWrite(IN3, 0); ledcWrite(IN4, robotSpeed); }
void turnRight()    { ledcWrite(IN1, robotSpeed); ledcWrite(IN2, 0); ledcWrite(IN3, 0); ledcWrite(IN4, robotSpeed); }
void turnLeft()     { ledcWrite(IN1, 0); ledcWrite(IN2, robotSpeed); ledcWrite(IN3, robotSpeed); ledcWrite(IN4, 0); }
void stopMotors()   { ledcWrite(IN1, 0); ledcWrite(IN2, 0); ledcWrite(IN3, 0); ledcWrite(IN4, 0); }

void executeCmd(char cmd) {
  // --- New Code Start: Overriding Block for Emergency Lock ---
  if (isEmergency) {
    stopMotors(); // Force stop immediately, ignore command
    return;
  }
  // --- New Code End ---

  if (cmd == 'F') moveForward();
  else if (cmd == 'B') moveBackward();
  else if (cmd == 'L') turnLeft();
  else if (cmd == 'R') turnRight();
  else if (cmd == 'S') stopMotors();
}

// --- Web Endpoints ---
void handleRoot()    { server.send(200, "text/html", INDEX_HTML); }
void handleCapture() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) { server.send(500, "text/plain", "Cam Fail"); return; }
  server.send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

void handleCommand() {
  if (server.hasArg("cmd")) {
    char cmd = server.arg("cmd")[0];
    if (currentMode == 'M' || currentMode == 'Y' || cmd == 'S') {
      executeCmd(cmd);
    }
  }
  server.send(200, "text/plain", "OK");
}

void handleMode() {
  if (server.hasArg("val")) {
    currentMode = server.arg("val")[0];
    stopMotors();
    // Do not change mode on Arduino if emergency is active
    if (!isEmergency) {
      Serial.print('M'); 
      Serial.print(currentMode); 
    }
  }
  server.send(200, "text/plain", "OK");
}

void handleSpeed() {
  if (server.hasArg("val")) {
    int reqSpeed = server.arg("val").toInt();
    robotSpeed = (reqSpeed < 150) ? 150 : reqSpeed;
    if (robotSpeed > 255) robotSpeed = 255;
  }
  server.send(200, "text/plain", "OK");
}

// --- New Code Start: API to Handle Emergency State ---
void handleEmergency() {
  if (server.hasArg("state")) {
    if (server.arg("state") == "1") {
      isEmergency = true;
      stopMotors();
      Serial.print('M'); 
      Serial.print('E'); // Tell Arduino to lock completely
    } else {
      isEmergency = false;
      Serial.print('M'); 
      Serial.print(currentMode); // Resume previous active mode on Arduino
    }
  }
  server.send(200, "text/plain", "OK");
}
// --- New Code End ---

void setup() {
  Serial.begin(9600); 

  setupMotors();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0; config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0=Y2_GPIO_NUM; config.pin_d1=Y3_GPIO_NUM; config.pin_d2=Y4_GPIO_NUM; config.pin_d3=Y5_GPIO_NUM;
  config.pin_d4=Y6_GPIO_NUM; config.pin_d5=Y7_GPIO_NUM; config.pin_d6=Y8_GPIO_NUM; config.pin_d7=Y9_GPIO_NUM;
  config.pin_xclk=XCLK_GPIO_NUM; config.pin_pclk=PCLK_GPIO_NUM; config.pin_vsync=VSYNC_GPIO_NUM; config.pin_href=HREF_GPIO_NUM;
  config.pin_sccb_sda=SIOD_GPIO_NUM; config.pin_sccb_scl=SIOC_GPIO_NUM; config.pin_pwdn=PWDN_GPIO_NUM; config.pin_reset=RESET_GPIO_NUM;
  config.xclk_freq_hz=20000000; config.pixel_format=PIXFORMAT_JPEG; config.frame_size=FRAMESIZE_VGA; config.jpeg_quality=12; config.fb_count=1;
  esp_camera_init(&config);

  WiFi.softAP(ssid, password);
  server.on("/", handleRoot);
  server.on("/capture", handleCapture);
  server.on("/action", handleCommand);
  server.on("/mode", handleMode);
  server.on("/speed", handleSpeed);
  
  // --- New Code Start: Register Emergency Endpoint ---
  server.on("/emergency", handleEmergency);
  // --- New Code End ---

  server.begin();
}

void loop() {
  server.handleClient();

  if ((currentMode == 'A' || currentMode == 'F') && Serial.available()) {
    char autoCmd = Serial.read();
    executeCmd(autoCmd);
  }
}