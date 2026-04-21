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

void setup() {
  Serial.begin(9600);     
  ESPSerial.begin(9600);  

  pinMode(LEFT_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);
  
  pinMode(FLAME_L, INPUT_PULLUP);
  pinMode(FLAME_F, INPUT_PULLUP);
  pinMode(FLAME_R, INPUT_PULLUP);
  
  pinMode(PUMP, OUTPUT); 
  digitalWrite(PUMP, HIGH); 
  
  pinMode(BUZZER, OUTPUT); 
  digitalWrite(BUZZER, LOW);
  
  waterServo.attach(SERVO_PIN);
  waterServo.write(90); 
}

void loop() {
  // --- Updated Serial Parsing (Much more stable) ---
  if (ESPSerial.available() > 0) {
    String msg = ESPSerial.readStringUntil('\n');
    msg.trim(); // Remove whitespace/newlines
    if (msg.startsWith("M:")) {
      sysMode = msg.charAt(2);
    }
  }

  // --- Global Emergency Lock Logic ---
  if (sysMode == 'E') {
    digitalWrite(PUMP, HIGH); 
    digitalWrite(BUZZER, LOW);
    ESPSerial.print('S'); 
    return; // Forcefully stops everything else from running
  }

  // --- 2. Line Follower Logic ---
  if (sysMode == 'A') {
    int leftValue = digitalRead(LEFT_SENSOR);
    int rightValue = digitalRead(RIGHT_SENSOR);

    if (leftValue == 0 && rightValue == 0) ESPSerial.print('F');
    else if (leftValue == 1 && rightValue == 0) ESPSerial.print('R');
    else if (leftValue == 0 && rightValue == 1) ESPSerial.print('L');
    else ESPSerial.print('S');
    
    delay(20); 
  }

  // --- 3. Firefighter Logic ---
  else if (sysMode == 'F') {
    int fL = digitalRead(FLAME_L);
    int fF = digitalRead(FLAME_F);
    int fR = digitalRead(FLAME_R);

    // Debouncing: Re-check after 5ms to avoid false triggers
    delay(5);
    if (fL == digitalRead(FLAME_L) || fF == digitalRead(FLAME_F) || fR == digitalRead(FLAME_R)) {
      if (fL == 0 || fF == 0 || fR == 0) {
        digitalWrite(BUZZER, HIGH); 
        ESPSerial.print('S'); 
        
        if (fF == 0) extinguish(90);
        else if (fL == 0) extinguish(160);
        else if (fR == 0) extinguish(20);
        
      } else {
        digitalWrite(BUZZER, LOW);     
        ESPSerial.print('S');
      }
    }
    delay(50);
  }
}

// --- Optimized Extinguish Function ---
void extinguish(int angle) {
  waterServo.write(angle); 
  delay(200); // Reduced delay for faster response
  digitalWrite(PUMP, LOW); 

  for(int i = angle - 10; i <= angle + 10; i++) { 
    waterServo.write(i); delay(10); 
  }
  for(int i = angle + 10; i >= angle - 10; i--) { 
    waterServo.write(i); delay(10); 
  }
  
  digitalWrite(PUMP, HIGH); 
  waterServo.write(90);    
}