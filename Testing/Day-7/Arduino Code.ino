#include <SoftwareSerial.h>
SoftwareSerial ESPSerial(10, 11); 

#define LEFT_SENSOR 4
#define RIGHT_SENSOR 5
#define FLAME_L 6
#define FLAME_F 7
#define FLAME_R 8
#define BUZZER 3

char sysMode = 'M'; 

// Non-blocking Variables
unsigned long previousLineTime = 0;
unsigned long previousFireSpamTime = 0;

// --- GLOBAL FIRE LOCK VARIABLE ---
bool fireAlarmLocked = false; 

void setup() {
  Serial.begin(9600);     
  ESPSerial.begin(9600);  

  pinMode(LED_BUILTIN, OUTPUT); 

  pinMode(LEFT_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);
  
  pinMode(FLAME_L, INPUT_PULLUP);
  pinMode(FLAME_F, INPUT_PULLUP);
  pinMode(FLAME_R, INPUT_PULLUP);
  
  pinMode(BUZZER, OUTPUT); 
  noTone(BUZZER); 
}

void loop() {
  unsigned long currentMillis = millis();


  while (ESPSerial.available() > 0) {
    char c = ESPSerial.read();
    

    if (c == 'M') {
      delay(10); 
      if (ESPSerial.available() >= 2) {
        char colon = ESPSerial.read();
        char modeChar = ESPSerial.read();
        
        if (colon == ':') {
      
          if (modeChar == 'A' || modeChar == 'F' || modeChar == 'M' || modeChar == 'E') {
            sysMode = modeChar;
            
         
            if (sysMode == 'E' || sysMode == 'M') {
              fireAlarmLocked = false; 
              noTone(BUZZER);
              digitalWrite(LED_BUILTIN, LOW);
            }
          }
        }
      }
    }
  }

 
  if (sysMode == 'A') {
    if (currentMillis - previousLineTime >= 100) { 
      int leftValue = digitalRead(LEFT_SENSOR);
      int rightValue = digitalRead(RIGHT_SENSOR);

      if (leftValue == 0 && rightValue == 0) ESPSerial.print('F');
      else if (leftValue == 1 && rightValue == 0) ESPSerial.print('R');
      else if (leftValue == 0 && rightValue == 1) ESPSerial.print('L');
      else ESPSerial.print('S');
      
      previousLineTime = currentMillis;
    }
  }


  else if (sysMode == 'F') {
    static int patrolPhase = 0;
    static unsigned long phaseTimer = 0;
    static unsigned long previousRadarTime = 0;


    if (!fireAlarmLocked) { 
      int fL = digitalRead(FLAME_L);
      int fF = digitalRead(FLAME_F);
      int fR = digitalRead(FLAME_R);
      
      if (fL == 0 || fF == 0 || fR == 0) {
        fireAlarmLocked = true; 
      }
    }


    if (fireAlarmLocked) {
      digitalWrite(LED_BUILTIN, HIGH); 
      tone(BUZZER, 1000);              
      

      if (currentMillis - previousFireSpamTime >= 100) {
        ESPSerial.print('S'); 
        previousFireSpamTime = currentMillis;
      }
    } 
    

    else {
      digitalWrite(LED_BUILTIN, LOW);  
      noTone(BUZZER);                  


      if (patrolPhase == 0) {
        if (currentMillis - previousRadarTime >= 100) {
          ESPSerial.print('F'); 
          previousRadarTime = currentMillis;
        }
        if (currentMillis - phaseTimer >= 4000) { 
          patrolPhase = 1; phaseTimer = currentMillis; 
        }
      }

      else if (patrolPhase == 1) {
        if (currentMillis - previousRadarTime >= 100) {
          ESPSerial.print('L'); 
          previousRadarTime = currentMillis;
        }
        if (currentMillis - phaseTimer >= 150) { 
          patrolPhase = 2; phaseTimer = currentMillis; 
        }
      }
 
      else if (patrolPhase == 2) {
        if (currentMillis - previousRadarTime >= 100) {
          ESPSerial.print('R'); 
          previousRadarTime = currentMillis;
        }
        if (currentMillis - phaseTimer >= 300) { 
          patrolPhase = 3; phaseTimer = currentMillis; 
        }
      }
   
      else if (patrolPhase == 3) {
        if (currentMillis - previousRadarTime >= 100) {
          ESPSerial.print('L'); 
          previousRadarTime = currentMillis;
        }
        if (currentMillis - phaseTimer >= 150) { 
          patrolPhase = 4; phaseTimer = currentMillis; 
        }
      }
     
      else if (patrolPhase == 4) {
        if (currentMillis - previousRadarTime >= 100) {
          ESPSerial.print('R'); 
          previousRadarTime = currentMillis;
        }
        if (currentMillis - phaseTimer >= 800) { 
          patrolPhase = 0; phaseTimer = currentMillis; 
        }
      }
    }
  }
}

