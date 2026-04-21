#include <SoftwareSerial.h>


SoftwareSerial ESPSerial(10, 11);


#define LEFT_SENSOR  4
#define RIGHT_SENSOR 5

char lastCmd = '\0';
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 80; // ms

void setup() {
  Serial.begin(9600);
  ESPSerial.begin(9600);
  pinMode(LEFT_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);
}

void loop() {
  int leftVal  = digitalRead(LEFT_SENSOR);
  int rightVal = digitalRead(RIGHT_SENSOR);

  char cmd;
  if      (leftVal == 0 && rightVal == 0) cmd = 'F'; 
  else if (leftVal == 1 && rightVal == 0) cmd = 'R';
  else if (leftVal == 0 && rightVal == 1) cmd = 'L'; 
  else                                    cmd = 'S'; 

  unsigned long now = millis();

  if (cmd != lastCmd || (now - lastSendTime >= SEND_INTERVAL)) {
    ESPSerial.print(cmd);
    lastCmd = cmd;
    lastSendTime = now;


    Serial.print("Cmd: "); Serial.println(cmd);
  }
}
