#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial ESPSerial(10, 11); 
Servo waterServo;

// --- Sensors ---
#define LEFT_SENSOR 4
#define RIGHT_SENSOR 5

#define FLAME_L 6
#define FLAME_F 7
#define FLAME_R 8

// --- Actuators ---
#define SERVO_PIN 9
#define RELAY_PIN 12   // Pump Relay
#define BUZZER 3

char sysMode = 'M';

// --- Timing ---
unsigned long prevLineTime = 0;
unsigned long prevFireTime = 0;

// --- Fire State ---
bool isExtinguishing = false;
int phase = 0;
unsigned long actionTime = 0;
int targetAngle = 90;
int currentAngle = 90;

void setup() {
  Serial.begin(9600);
  ESPSerial.begin(9600);

  pinMode(LEFT_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);

  pinMode(FLAME_L, INPUT_PULLUP);
  pinMode(FLAME_F, INPUT_PULLUP);
  pinMode(FLAME_R, INPUT_PULLUP);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Pump OFF (IMPORTANT)

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  waterServo.attach(SERVO_PIN);
  waterServo.write(90);
}

void loop() {
  unsigned long now = millis();

  // --- Serial Sync ---
  if (ESPSerial.available()) {
    String msg = ESPSerial.readStringUntil('\n');
    msg.trim();

    if (msg.startsWith("M:")) {
      sysMode = msg.charAt(2);

      if (sysMode == 'E' || sysMode == 'M') {
        stopAll();
      }
    }
  }

  // --- Emergency ---
  if (sysMode == 'E') {
    stopAll();
    ESPSerial.print('S');
    return;
  }

  // --- Fire State Machine ---
  if (isExtinguishing) {
    handleFire(now);
    return;
  }

  // --- Line Follow ---
  if (sysMode == 'A') {
    if (now - prevLineTime > 20) {
      int L = digitalRead(LEFT_SENSOR);
      int R = digitalRead(RIGHT_SENSOR);

      if (L == 0 && R == 0) ESPSerial.print('F');
      else if (L == 1 && R == 0) ESPSerial.print('R');
      else if (L == 0 && R == 1) ESPSerial.print('L');
      else ESPSerial.print('S');

      prevLineTime = now;
    }
  }

  // --- Fire Detection ---
  else if (sysMode == 'F') {
    int fL = digitalRead(FLAME_L);
    int fF = digitalRead(FLAME_F);
    int fR = digitalRead(FLAME_R);

    if (fL == 0 || fF == 0 || fR == 0) {
      digitalWrite(BUZZER, HIGH);
      ESPSerial.print('S');

      isExtinguishing = true;
      phase = 1;
      actionTime = now;

      if (fF == 0) targetAngle = 90;
      else if (fL == 0) targetAngle = 160;
      else targetAngle = 20;

      waterServo.write(targetAngle);
      currentAngle = targetAngle - 10;
    } 
    else {
      digitalWrite(BUZZER, LOW);

      if (now - prevFireTime > 50) {
        ESPSerial.print('S');
        prevFireTime = now;
      }
    }
  }
}

// Fire Extinguish Logic
void handleFire(unsigned long now) {

  // Aim delay
  if (phase == 1 && now - actionTime > 200) {
    digitalWrite(RELAY_PIN, LOW); // Pump ON
    phase = 2;
    actionTime = now;
  }

  // Sweep forward
  else if (phase == 2 && now - actionTime > 10) {
    currentAngle++;
    waterServo.write(currentAngle);
    actionTime = now;

    if (currentAngle >= targetAngle + 10)
      phase = 3;
  }

  // Sweep backward
  else if (phase == 3 && now - actionTime > 10) {
    currentAngle--;
    waterServo.write(currentAngle);
    actionTime = now;

    if (currentAngle <= targetAngle - 10)
      phase = 4;
  }

  // Finish
  else if (phase == 4) {
    digitalWrite(RELAY_PIN, HIGH); // Pump OFF
    waterServo.write(90);

    isExtinguishing = false;
    phase = 0;
  }
}

// Emergency Stop
void stopAll() {
  digitalWrite(RELAY_PIN, HIGH); // Pump OFF
  digitalWrite(BUZZER, LOW);
  waterServo.write(90);

  isExtinguishing = false;
  phase = 0;
}