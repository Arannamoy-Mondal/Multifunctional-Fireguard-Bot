#include <SoftwareSerial.h>

// ESP32-CAM এর GPIO1(Tx)/GPIO3(Rx) এর সাথে সংযোগ
// Arduino pin 10 → ESP32 GPIO3 (Rx)
// Arduino pin 11 → ESP32 GPIO1 (Tx)  [ঐচ্ছিক, এখানে প্রয়োজন নেই]
SoftwareSerial ESPSerial(10, 11);

// --- IR সেন্সর পিন ---
#define LEFT_SENSOR  4
#define RIGHT_SENSOR 5

// --- State tracking (flood prevention) ---
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

  // লাইন ফলোয়িং লজিক
  // 0 = কালো লাইন সনাক্ত, 1 = লাইনের বাইরে
  char cmd;
  if      (leftVal == 0 && rightVal == 0) cmd = 'F'; // সামনে
  else if (leftVal == 1 && rightVal == 0) cmd = 'R'; // ডানে ঘোরো
  else if (leftVal == 0 && rightVal == 1) cmd = 'L'; // বামে ঘোরো
  else                                    cmd = 'S'; // লাইনের বাইরে, থামো

  unsigned long now = millis();

  // শুধু তখনই পাঠাও যখন: কমান্ড বদলেছে অথবা interval পেরিয়েছে
  if (cmd != lastCmd || (now - lastSendTime >= SEND_INTERVAL)) {
    ESPSerial.print(cmd);
    lastCmd = cmd;
    lastSendTime = now;

    // ডিবাগ
    Serial.print("Cmd: "); Serial.println(cmd);
  }
}
