#include <SoftwareSerial.h>

// ESP32-CAM এর কাছে ডেটা পাঠানোর জন্য (Rx: 10, Tx: 11)
SoftwareSerial ESPSerial(10, 11); 

// --- IR সেন্সর পিন ---
#define LEFT_SENSOR 4
#define RIGHT_SENSOR 5

void setup() {
  Serial.begin(9600);     // পিসিতে ডিবাগ করার জন্য
  ESPSerial.begin(9600);  // ESP32-CAM এ ডেটা পাঠানোর জন্য

  pinMode(LEFT_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);
}

void loop() {
  // ডিজিটাল ডেটা রিড করা (0 মানে কালো লাইন, 1 মানে সাদা অংশ - সেন্সরের ওপর ভিত্তি করে উল্টোও হতে পারে)
  int leftValue = digitalRead(LEFT_SENSOR);
  int rightValue = digitalRead(RIGHT_SENSOR);

  // লজিক: দুটি সেন্সরই কালো লাইনের ওপর থাকলে সামনে যাবে
  if (leftValue == 0 && rightValue == 0) {
    ESPSerial.print('F');
  } 
  // বামের সেন্সর সাদা অংশে গেলে, ডান দিকে ঘুরতে হবে
  else if (leftValue == 1 && rightValue == 0) {
    ESPSerial.print('R');
  } 
  // ডানের সেন্সর সাদা অংশে গেলে, বাম দিকে ঘুরতে হবে
  else if (leftValue == 0 && rightValue == 1) {
    ESPSerial.print('L');
  } 
  // লাইনের বাইরে চলে গেলে থেমে যাবে
  else {
    ESPSerial.print('S');
  }

  // ডেটা ফ্লাডিং কমানোর জন্য হালকা ডিলে
  delay(100); 
}
