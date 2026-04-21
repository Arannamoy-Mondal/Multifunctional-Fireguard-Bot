#include <SoftwareSerial.h>


SoftwareSerial ESPSerial(10, 11); 


#define LEFT_SENSOR 4
#define RIGHT_SENSOR 5

void setup() {
  Serial.begin(9600);    
  ESPSerial.begin(9600); 

  pinMode(LEFT_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);
}

void loop() {

  int leftValue = digitalRead(LEFT_SENSOR);
  int rightValue = digitalRead(RIGHT_SENSOR);

  if (leftValue == 0 && rightValue == 0) {
    ESPSerial.print('F');
  } 
  
  else if (leftValue == 1 && rightValue == 0) {
    ESPSerial.print('R');
  } 

  else if (leftValue == 0 && rightValue == 1) {
    ESPSerial.print('L');
  } 

  else {
    ESPSerial.print('S');
  }


  delay(100); 
}
