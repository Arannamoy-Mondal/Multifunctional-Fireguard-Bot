#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial ESPSerial(10, 11); 
Servo waterServo;

#define LEFT_SENSOR 4
#define RIGHT_SENSOR 5
#define FLAME_L 6
#define FLAME_F 7
#define FLAME_R 8
#define SERVO_PIN 9
#define PUMP 12
#define BUZZER 3

#define PUMP_ON  LOW
#define PUMP_OFF HIGH

char sysMode = 'M'; 

// Non-blocking Variables
unsigned long previousLineTime = 0;
unsigned long previousFireSpamTime = 0;
unsigned long previousEmergencyTime = 0;

// Fire Extinguishing State Machine Variables
bool isExtinguishing = false;
int extinguishPhase = 0; 
unsigned long extinguishTimer = 0;
int targetAngle = 90;
int currentAngle = 90;

// Pulse Pump Variables
unsigned long lastPumpPulse = 0;
bool pumpState = false;

// Debouncing Variables
bool potentialFire = false;
unsigned long fireDetectTime = 0;

void setup() {
  Serial.begin(9600);     
  ESPSerial.begin(9600);  

  pinMode(LEFT_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);
  pinMode(FLAME_L, INPUT_PULLUP);
  pinMode(FLAME_F, INPUT_PULLUP);
  pinMode(FLAME_R, INPUT_PULLUP);
  pinMode(PUMP, OUTPUT); 
  digitalWrite(PUMP, PUMP_OFF); 
  pinMode(BUZZER, OUTPUT); 
  digitalWrite(BUZZER, LOW);
  
  waterServo.attach(SERVO_PIN);
  waterServo.write(90); 
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. ALWAYS LISTEN TO ESP32
  if (ESPSerial.available() > 0) {
    String msg = ESPSerial.readStringUntil('\n');
    msg.trim(); 
    if (msg.startsWith("M:")) {
      sysMode = msg.charAt(2);
      if (sysMode == 'E' || sysMode == 'M') {
        isExtinguishing = false;
        extinguishPhase = 0;
        digitalWrite(PUMP, PUMP_OFF); 
        digitalWrite(BUZZER, LOW);
      }
    }
  }

  // 2. GLOBAL EMERGENCY LOCK
  if (sysMode == 'E') {
    if (currentMillis - previousEmergencyTime >= 100) {
      digitalWrite(PUMP, PUMP_OFF); 
      digitalWrite(BUZZER, LOW);
      ESPSerial.print('S'); 
      previousEmergencyTime = currentMillis;
    }
    return; 
  }

  // 3. HANDLE ACTIVE FIRE EXTINGUISHING
  if (isExtinguishing) {
    handleExtinguishing(currentMillis);
    return; 
  }

  // 4. LINE FOLLOWER LOGIC
  if (sysMode == 'A') {
    if (currentMillis - previousLineTime >= 20) {
      int leftValue = digitalRead(LEFT_SENSOR);
      int rightValue = digitalRead(RIGHT_SENSOR);

      if (leftValue == 0 && rightValue == 0) ESPSerial.print('F');
      else if (leftValue == 1 && rightValue == 0) ESPSerial.print('R');
      else if (leftValue == 0 && rightValue == 1) ESPSerial.print('L');
      else ESPSerial.print('S');
      
      previousLineTime = currentMillis;
    }
  }

  // 5. FIREFIGHTER LOGIC
  else if (sysMode == 'F') {
    int fL = digitalRead(FLAME_L);
    int fF = digitalRead(FLAME_F);
    int fR = digitalRead(FLAME_R);

    if (fL == 0 || fF == 0 || fR == 0) {
      if (!potentialFire) {
        potentialFire = true;
        fireDetectTime = currentMillis; 
      } 
      else if (currentMillis - fireDetectTime >= 5) {
        digitalWrite(BUZZER, HIGH); 
        ESPSerial.print('S'); 
        
        isExtinguishing = true;
        extinguishPhase = 1; 
        extinguishTimer = currentMillis;
        lastPumpPulse = currentMillis;
        pumpState = true;
        
        if (fF == 0) targetAngle = 90;
        else if (fL == 0) targetAngle = 140; // Adjusted for smoother track
        else if (fR == 0) targetAngle = 40;  // Adjusted for smoother track

        waterServo.write(targetAngle);
        currentAngle = targetAngle - 15; 
        potentialFire = false;
      }
    } 
    else {
      potentialFire = false;
      digitalWrite(BUZZER, LOW); 
      if (currentMillis - previousFireSpamTime >= 50) {
        ESPSerial.print('S');
        previousFireSpamTime = currentMillis;
      }
    }
  }
}

// --- SMART PULSE SPRAY & SWEEP ---
void handleExtinguishing(unsigned long currentMillis) {
  // Phase 1: Aiming
  if (extinguishPhase == 1) {
    if (currentMillis - extinguishTimer >= 200) {
      extinguishTimer = currentMillis;
      extinguishPhase = 2; 
    }
  }
  // Phase 2 & 3: Sweeping and Pulse Spraying
  else if (extinguishPhase == 2 || extinguishPhase == 3) {
    
    // Pulse Pump Logic (100ms ON/OFF interval)
    if (currentMillis - lastPumpPulse >= 100) {
      pumpState = !pumpState;
      digitalWrite(PUMP, pumpState ? PUMP_ON : PUMP_OFF);
      lastPumpPulse = currentMillis;
    }

    // Servo Sweep Logic
    if (currentMillis - extinguishTimer >= 15) { 
      if (extinguishPhase == 2) {
        currentAngle++;
        if (currentAngle >= targetAngle + 15) extinguishPhase = 3;
      } else {
        currentAngle--;
        if (currentAngle <= targetAngle - 15) extinguishPhase = 4;
      }
      waterServo.write(currentAngle);
      extinguishTimer = currentMillis;
    }
  }
  // Phase 4: Reset
  else if (extinguishPhase == 4) {
    digitalWrite(PUMP, PUMP_OFF); 
    waterServo.write(90);     
    isExtinguishing = false;  
    extinguishPhase = 0;
  }
}