const int IR_SENSOR = 14; 
const int LED_PIN = 2;

void setup() {

  Serial.begin(115200);
  pinMode(IR_SENSOR, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("FC-51 Sensor Test - Single Unit");
}

void loop() {
  int status = digitalRead(IR_SENSOR);

  if (status == LOW) {
    digitalWrite(LED_PIN, HIGH);
  } 
  else {
  
    digitalWrite(LED_PIN, LOW);
  }
  delay(100); 
}