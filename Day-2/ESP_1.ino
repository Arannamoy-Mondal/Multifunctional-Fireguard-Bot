#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "FireBot";
const char* password = ""; 

WebServer server(80);

#define IN1 14 
#define IN2 15 
#define IN3 13 
#define IN4 12 

int robotSpeed = 200; 
char currentMode = 'M'; // M: Manual, A: Auto/Line, F: Firefighting

// Camera Pins Ai-Thinker
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>FireBot Elite</title>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background-color: #2c3e50; color: white; margin: 0; padding: 10px; }
    img { width: 100%; max-width: 600px; border-radius: 10px; border: 4px solid #34495e; }
    .tabs { display: flex; justify-content: center; gap: 5px; margin: 15px 0; }
    .tab-btn { flex: 1; padding: 10px 2px; font-size: 11px; font-weight: bold; border: none; border-radius: 5px; color: white; background-color: #7f8c8d; cursor: pointer; }
    .control-section { display: none; }
    .active-section { display: block; }
    .pad { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; max-width: 320px; margin: 0 auto; }
    .btn { background-color: #3498db; color: white; border: none; padding: 15px; font-weight: bold; border-radius: 10px; }
    .btn-red { background-color: #e74c3c; }
    .status { padding: 20px; background: #34495e; border-radius: 10px; margin: 10px auto; max-width: 300px; }
  </style>
</head>
<body onload="switchTab('M')">
  <h2>FIREBOT PRO</h2>
  <img id="camFeed" src="/capture">
  
  <div class="tabs">
    <button id="tabM" class="tab-btn" onclick="switchTab('M')">MANUAL</button>
    <button id="tabA" class="tab-btn" onclick="switchTab('A')">LINE FOLLOW</button>
    <button id="tabF" class="tab-btn" onclick="switchTab('F')">FIREFIGHTER</button>
  </div>
  
  <div id="secM" class="control-section">
    <div class="pad">
      <div></div><button class="btn" onmousedown="sendCmd('F')" onmouseup="sendCmd('S')">FORWARD</button><div></div>
      <button class="btn" onmousedown="sendCmd('L')" onmouseup="sendCmd('S')">LEFT</button>
      <button class="btn btn-red" onclick="sendCmd('S')">STOP</button>
      <button class="btn" onmousedown="sendCmd('R')" onmouseup="sendCmd('S')">RIGHT</button>
      <div></div><button class="btn" onmousedown="sendCmd('B')" onmouseup="sendCmd('S')">BACKWARD</button><div></div>
    </div>
  </div>

  <div id="secA" class="control-section"><div class="status">LINE FOLLOWER ACTIVE</div></div>
  <div id="secF" class="control-section"><div class="status" style="color:#e67e22">FIREFIGHTING MODE ACTIVE</div></div>

  <script>
    function switchTab(m) {
      document.querySelectorAll('.control-section').forEach(s => s.classList.remove('active-section'));
      document.querySelectorAll('.tab-btn').forEach(b => b.style.backgroundColor = '#7f8c8d');
      if(m=='M'){ document.getElementById('secM').classList.add('active-section'); document.getElementById('tabM').style.backgroundColor='#e67e22'; }
      else if(m=='A'){ document.getElementById('secA').classList.add('active-section'); document.getElementById('tabA').style.backgroundColor='#27ae60'; }
      else if(m=='F'){ document.getElementById('secF').classList.add('active-section'); document.getElementById('tabF').style.backgroundColor='#c0392b'; }
      fetch('/mode?val=' + m);
    }
    function sendCmd(c) { fetch('/action?cmd=' + c); }
    setInterval(function() { document.getElementById('camFeed').src = '/capture?r=' + Math.random(); }, 150);
  </script>
</body>
</html>
)rawliteral";

void setupMotors() {
  ledcAttach(IN1, 2000, 8); ledcAttach(IN2, 2000, 8);
  ledcAttach(IN3, 2000, 8); ledcAttach(IN4, 2000, 8);
}

void moveForward() { ledcWrite(IN1, robotSpeed); ledcWrite(IN2, 0); ledcWrite(IN3, robotSpeed); ledcWrite(IN4, 0); }
void moveBackward() { ledcWrite(IN1, 0); ledcWrite(IN2, robotSpeed); ledcWrite(IN3, 0); ledcWrite(IN4, robotSpeed); }
void turnRight() { ledcWrite(IN1, robotSpeed); ledcWrite(IN2, 0); ledcWrite(IN3, 0); ledcWrite(IN4, robotSpeed); }
void turnLeft() { ledcWrite(IN1, 0); ledcWrite(IN2, robotSpeed); ledcWrite(IN3, robotSpeed); ledcWrite(IN4, 0); }
void stopMotors() { ledcWrite(IN1, 0); ledcWrite(IN2, 0); ledcWrite(IN3, 0); ledcWrite(IN4, 0); }

void handleCommand() {
  if (server.hasArg("cmd")) {
    char cmd = server.arg("cmd")[0];
    if (currentMode == 'M' || cmd == 'S') {
      if (cmd == 'F') moveForward(); else if (cmd == 'B') moveBackward();
      else if (cmd == 'L') turnLeft(); else if (cmd == 'R') turnRight();
      else if (cmd == 'S') stopMotors();
    }
  }
  server.send(200, "text/plain", "OK");
}

void handleMode() {
  if (server.hasArg("val")) {
    currentMode = server.arg("val")[0];
    stopMotors();
    Serial.print("M"); Serial.println(currentMode); 
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(9600);
  setupMotors();
  // Camera Init (Standard configuration)
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0; config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0=Y2_GPIO_NUM; config.pin_d1=Y3_GPIO_NUM; config.pin_d2=Y4_GPIO_NUM; config.pin_d3=Y5_GPIO_NUM;
  config.pin_d4=Y6_GPIO_NUM; config.pin_d5=Y7_GPIO_NUM; config.pin_d6=Y8_GPIO_NUM; config.pin_d7=Y9_GPIO_NUM;
  config.pin_xclk=XCLK_GPIO_NUM; config.pin_pclk=PCLK_GPIO_NUM; config.pin_vsync=VSYNC_GPIO_NUM; config.pin_href=HREF_GPIO_NUM;
  config.pin_sccb_sda=SIOD_GPIO_NUM; config.pin_sccb_scl=SIOC_GPIO_NUM; config.pin_pwdn=PWDN_GPIO_NUM; config.pin_reset=RESET_GPIO_NUM;
  config.xclk_freq_hz=20000000; config.pixel_format=PIXFORMAT_JPEG; config.frame_size=FRAMESIZE_VGA; config.jpeg_quality=12; config.fb_count=1;
  esp_camera_init(&config);

  WiFi.softAP(ssid, password);
  server.on("/", handleRoot); server.on("/capture", handleCapture);
  server.on("/action", handleCommand); server.on("/mode", handleMode);
  server.begin();
}

void loop() {
  server.handleClient();
  if (currentMode != 'M' && Serial.available()) {
    char autoCmd = Serial.read();
    if (autoCmd == 'F') moveForward(); else if (autoCmd == 'L') turnLeft();
    else if (autoCmd == 'R') turnRight(); else if (autoCmd == 'S') stopMotors();
  }
}
void handleRoot() { server.send(200, "text/html", INDEX_HTML); }
void handleCapture() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) { server.send(500, "text/plain", "Cam Fail"); return; }
  server.send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}
