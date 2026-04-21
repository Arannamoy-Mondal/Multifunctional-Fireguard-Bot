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

char sysMode = 'M'; 

// --- Non-blocking millis() Variables ---
unsigned long previousLineTime = 0;
unsigned long previousFireSpamTime = 0;
unsigned long previousEmergencyTime = 0;

// Fire Extinguishing State Machine Variables
bool isExtinguishing = false;
int extinguishPhase = 0; 
unsigned long extinguishTimer = 0;
int targetAngle = 90;
int currentAngle = 90;

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
  digitalWrite(PUMP, HIGH); // Relay OFF (Active LOW)
  
  pinMode(BUZZER, OUTPUT); 
  digitalWrite(BUZZER, LOW);
  
  waterServo.attach(SERVO_PIN);
  waterServo.write(90); 
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. ALWAYS LISTEN TO ESP32 (Zero Delay) ---
  if (ESPSerial.available() > 0) {
    String msg = ESPSerial.readStringUntil('\n');
    msg.trim(); 
    if (msg.startsWith("M:")) {
      sysMode = msg.charAt(2);
      
      // If switched to Emergency or Manual, forcefully reset any active fire extinguishing
      if (sysMode == 'E' || sysMode == 'M') {
        isExtinguishing = false;
        extinguishPhase = 0;
        digitalWrite(PUMP, HIGH);
        digitalWrite(BUZZER, LOW);
      }
    }
  }

  // --- 2. GLOBAL EMERGENCY LOCK ---
  if (sysMode == 'E') {
    // Send stop signal every 100ms to avoid spamming the serial buffer
    if (currentMillis - previousEmergencyTime >= 100) {
      digitalWrite(PUMP, HIGH); 
      digitalWrite(BUZZER, LOW);
      ESPSerial.print('S'); 
      previousEmergencyTime = currentMillis;
    }
    return; // Block everything else below this line!
  }

  // --- 3. HANDLE ACTIVE FIRE EXTINGUISHING (State Machine) ---
  if (isExtinguishing) {
    handleExtinguishing(currentMillis);
    return; // Don't do line following or start a new fire check while already spraying
  }

  // --- 4. LINE FOLLOWER LOGIC (Runs every 20ms) ---
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

  // --- 5. FIREFIGHTER LOGIC (Non-blocking Debounce & Detection) ---
  else if (sysMode == 'F') {
    int fL = digitalRead(FLAME_L);
    int fF = digitalRead(FLAME_F);
    int fR = digitalRead(FLAME_R);

    // If any fire sensor triggers
    if (fL == 0 || fF == 0 || fR == 0) {
      if (!potentialFire) {
        potentialFire = true;
        fireDetectTime = currentMillis; // Start debounce timer
      } 
      // If fire is consistent for 5ms (Debounced)
      else if (currentMillis - fireDetectTime >= 5) {
        digitalWrite(BUZZER, HIGH); 
        ESPSerial.print('S'); // Stop the bot
        
        // Setup the State Machine for extinguishing
        isExtinguishing = true;
        extinguishPhase = 1; 
        extinguishTimer = currentMillis;
        
        if (fF == 0) targetAngle = 90;
        else if (fL == 0) targetAngle = 160;
        else if (fR == 0) targetAngle = 20;

        waterServo.write(targetAngle);
        currentAngle = targetAngle - 10; // Prepare for sweep start
        potentialFire = false;
      }
    } 
    else {
      // No fire detected
      potentialFire = false;
      digitalWrite(BUZZER, LOW); 
      
      // Stop moving while scanning, send 'S' every 50ms
      if (currentMillis - previousFireSpamTime >= 50) {
        ESPSerial.print('S');
        previousFireSpamTime = currentMillis;
      }
    }
  }
}

// --- NON-BLOCKING SERVO SWEEP FUNCTION ---
void handleExtinguishing(unsigned long currentMillis) {
  // Phase 1: Pre-delay (Aiming at fire for 200ms before shooting water)
  if (extinguishPhase == 1) {
    if (currentMillis - extinguishTimer >= 200) {
      digitalWrite(PUMP, LOW); // Turn PUMP ON
      waterServo.write(currentAngle);
      extinguishTimer = currentMillis;
      extinguishPhase = 2; // Move to next phase
    }
  }
  // Phase 2: Sweep Forward (+20 degrees)
  else if (extinguishPhase == 2) {
    if (currentMillis - extinguishTimer >= 10) { // 10ms per step
      currentAngle++;
      waterServo.write(currentAngle);
      extinguishTimer = currentMillis;
      
      if (currentAngle >= targetAngle + 10) {
        extinguishPhase = 3;
      }
    }
  }
  // Phase 3: Sweep Backward (-20 degrees)
  else if (extinguishPhase == 3) {
    if (currentMillis - extinguishTimer >= 10) {
      currentAngle--;
      waterServo.write(currentAngle);
      extinguishTimer = currentMillis;
      
      if (currentAngle <= targetAngle - 10) {
        extinguishPhase = 4; // Finished sweeping
      }
    }
  }
  // Phase 4: Clean up and Reset
  else if (extinguishPhase == 4) {
    digitalWrite(PUMP, HIGH); // Turn PUMP OFF
    waterServo.write(90);     // Return to center
    isExtinguishing = false;  // Exit state machine
    extinguishPhase = 0;
  }
}