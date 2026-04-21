#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial ESPSerial(10, 11); 
Servo waterServo;

#define LEFT_IR 4
#define RIGHT_IR 5
#define FLAME_LEFT 6
#define FLAME_FRONT 7
#define FLAME_RIGHT 8
#define PUMP 12
#define SERVO_PIN 9
#define BUZZER_PIN 13  

char systemMode = 'M'; 

void setup() {
  Serial.begin(9600);
  ESPSerial.begin(9600);
  
  pinMode(LEFT_IR, INPUT); pinMode(RIGHT_IR, INPUT);
  pinMode(FLAME_LEFT, INPUT); pinMode(FLAME_FRONT, INPUT); pinMode(FLAME_RIGHT, INPUT);
  pinMode(PUMP, OUTPUT); digitalWrite(PUMP, HIGH); 
  pinMode(BUZZER_PIN, OUTPUT); 
  
  waterServo.attach(SERVO_PIN);
  waterServo.write(90); 
}

void loop() {
  
  if (ESPSerial.available() > 1) {
    if (ESPSerial.read() == 'M') {
      systemMode = ESPSerial.read();
    }
  }

  if (systemMode == 'A') {
    lineFollowerLogic();
  } 
  else if (systemMode == 'F') {
    firefighterLogic();
  }
}

void lineFollowerLogic() {
  int L = digitalRead(LEFT_IR);
  int R = digitalRead(RIGHT_IR);
  if (L == 0 && R == 0) ESPSerial.print('F');
  else if (L == 1 && R == 0) ESPSerial.print('R');
  else if (L == 0 && R == 1) ESPSerial.print('L');
  else ESPSerial.print('S');
  delay(50);
}

void firefighterLogic() {
  int fL = digitalRead(FLAME_LEFT);
  int fF = digitalRead(FLAME_FRONT);
  int fR = digitalRead(FLAME_RIGHT);


  if (fL == 0 || fF == 0 || fR == 0) {
    digitalWrite(BUZZER_PIN, HIGH); 
  } else {
    digitalWrite(BUZZER_PIN, LOW); 
  }

  
  if (fF == 0) {
    ESPSerial.print('S'); 
    extinguishFire(90); 
  } 
  else if (fL == 0) {
    ESPSerial.print('S'); 
    extinguishFire(160); 
  } 
  else if (fR == 0) {
    ESPSerial.print('S'); 
    extinguishFire(20);  
  } 
  else {
    ESPSerial.print('S');
  }
  delay(100);
}

void extinguishFire(int targetAngle) {
  waterServo.write(targetAngle); 
  delay(500); 
  digitalWrite(PUMP, LOW); 
  
  
  for (int p = targetAngle - 15; p <= targetAngle + 15; p++) {
    waterServo.write(p); delay(20);
  }
  for (int p = targetAngle + 15; p >= targetAngle - 15; p--) {
    waterServo.write(p); delay(20);
  }
  
  digitalWrite(PUMP, HIGH); 
  waterServo.write(90); 
}
