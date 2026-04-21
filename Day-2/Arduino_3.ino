// #include <SoftwareSerial.h>
// #include <Servo.h>

// SoftwareSerial ESPSerial(10, 11); 
// Servo waterServo;

// #define LEFT_SENSOR 4
// #define RIGHT_SENSOR 5

// #define FLAME_L 6
// #define FLAME_F 7
// #define FLAME_R 8
// #define SERVO_PIN 9
// #define PUMP 12
// #define BUZZER 13

// char sysMode = 'M'; // Default

// // void setup() {
// //   Serial.begin(9600);     
// //   ESPSerial.begin(9600);  

// //   pinMode(LEFT_SENSOR, INPUT);
// //   pinMode(RIGHT_SENSOR, INPUT);
  
// //   pinMode(FLAME_L, INPUT);
// //   pinMode(FLAME_F, INPUT);
// //   pinMode(FLAME_R, INPUT);
  
// //   pinMode(PUMP, OUTPUT); 
// //   digitalWrite(PUMP, HIGH); // Relay default OFF
  
// //   pinMode(BUZZER, OUTPUT); 
// //   digitalWrite(BUZZER, LOW);
  
// //   waterServo.attach(SERVO_PIN);
// //   waterServo.write(90);
// // }

// void setup() {
//   Serial.begin(9600);     
//   ESPSerial.begin(9600);  

//   pinMode(LEFT_SENSOR, INPUT);
//   pinMode(RIGHT_SENSOR, INPUT);
  
//   // শুধু এই তিনটিতে INPUT_PULLUP করে দিন
//   pinMode(FLAME_L, INPUT_PULLUP);
//   pinMode(FLAME_F, INPUT_PULLUP);
//   pinMode(FLAME_R, INPUT_PULLUP);
  
//   pinMode(PUMP, OUTPUT); 
//   digitalWrite(PUMP, HIGH); 
  
//   pinMode(BUZZER, OUTPUT); 
//   digitalWrite(BUZZER, LOW);
  
//   waterServo.attach(SERVO_PIN);
//   waterServo.write(90);
// }

// void loop() {
//   // Read mode from ESP32
//   if (ESPSerial.available() > 0) {
//     char incoming = ESPSerial.read();
//     if (incoming == 'M') {
//       delay(10);
//       if (ESPSerial.available() > 0) {
//         sysMode = ESPSerial.read();
//       }
//     }
//   }

//   // --- Line Follower Logic (exactly as your working code) ---
//   if (sysMode == 'A') {
//     int leftValue = digitalRead(LEFT_SENSOR);
//     int rightValue = digitalRead(RIGHT_SENSOR);

//     if (leftValue == 0 && rightValue == 0) {
//       ESPSerial.print('F');
//     } 
//     else if (leftValue == 1 && rightValue == 0) {
//       ESPSerial.print('R');
//     } 
//     else if (leftValue == 0 && rightValue == 1) {
//       ESPSerial.print('L');
//     } 
//     else {
//       ESPSerial.print('S');
//     }
//     delay(100); // Maintained your delay
//   }

//   // --- Firefighter Logic ---
//   else if (sysMode == 'F') {
//     int fL = digitalRead(FLAME_L);
//     int fF = digitalRead(FLAME_F);
//     int fR = digitalRead(FLAME_R);

//     if (fL == 0 || fF == 0 || fR == 0) {
//       digitalWrite(BUZZER, HIGH);
//       ESPSerial.print('S'); // Stop wheels
      
//       if (fF == 0) extinguish(90);
//       else if (fL == 0) extinguish(160);
//       else if (fR == 0) extinguish(20);
      
//     } else {
//       digitalWrite(BUZZER, LOW);
//       ESPSerial.print('S');
//     }
//     delay(100);
//   }
// }

// // Function to spray water
// void extinguish(int angle) {
//   waterServo.write(angle); 
//   delay(500); 
//   digitalWrite(PUMP, LOW);
  
//   // Sweep servo
//   for(int i = angle - 15; i <= angle + 15; i++) { 
//     waterServo.write(i); delay(20); 
//   }
//   for(int i = angle + 15; i >= angle - 15; i--) { 
//     waterServo.write(i); delay(20); 
//   }
  
//   digitalWrite(PUMP, HIGH);
//   waterServo.write(90); // Return to center
// }


#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial ESPSerial(10, 11); 
Servo waterServo;

// --- সেন্সর ও পিন কনফিগারেশন ---
#define LEFT_SENSOR 4
#define RIGHT_SENSOR 5

#define FLAME_L 6
#define FLAME_F 7
#define FLAME_R 8
#define SERVO_PIN 9
#define PUMP 12
#define BUZZER 3

char sysMode = 'M'; // ডিফল্ট মোড

void setup() {
  Serial.begin(9600);     
  ESPSerial.begin(9600);  

  pinMode(LEFT_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);
  
  // ফ্লেম সেন্সরগুলোতে INPUT_PULLUP দেওয়া হয়েছে (যাতে সেন্সর খোলা থাকলে বাজার না বাজে)
  pinMode(FLAME_L, INPUT_PULLUP);
  pinMode(FLAME_F, INPUT_PULLUP);
  pinMode(FLAME_R, INPUT_PULLUP);
  
  pinMode(PUMP, OUTPUT); 
  digitalWrite(PUMP, HIGH); // রিলে ডিফল্ট অফ
  
  pinMode(BUZZER, OUTPUT); 
  digitalWrite(BUZZER, LOW);
  
  waterServo.attach(SERVO_PIN);
  waterServo.write(90); // সেন্টারে পজিশন
}

void loop() {
  // ১. ESP32 থেকে মোড রিসিভ করা
  if (ESPSerial.available() > 0) {
    char incoming = ESPSerial.read();
    if (incoming == 'M') {
      delay(10);
      if (ESPSerial.available() > 0) {
        sysMode = ESPSerial.read();
      }
    }
  }

  // --- ২. Line Follower Logic ---
  if (sysMode == 'A') {
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

  // --- ৩. Firefighter Logic ---
  else if (sysMode == 'F') {
    int fL = digitalRead(FLAME_L);
    int fF = digitalRead(FLAME_F);
    int fR = digitalRead(FLAME_R);

    if (fL == 0 || fF == 0 || fR == 0) {
      tone(BUZZER, 1000);   // Passive Buzzer এর জন্য 1000Hz ফ্রিকোয়েন্সিতে শব্দ
      ESPSerial.print('S'); // চাকা থামানোর কমান্ড
      
      // কোন দিকের সেন্সর আগুন পেয়েছে সেই অনুযায়ী পানি ছিটানো
      if (fF == 0) extinguish(90);
      else if (fL == 0) extinguish(160);
      else if (fR == 0) extinguish(20);
      
    } else {
      noTone(BUZZER);       // আগুন না থাকলে বাজার পুরোপুরি বন্ধ
      ESPSerial.print('S');
    }
    delay(100);
  }
}

// --- পানি ছিটানোর ফাংশন (Sweep Servo) ---
void extinguish(int angle) {
  waterServo.write(angle); 
  delay(500); 
  digitalWrite(PUMP, LOW); // পাম্প চালু
  
  // ডানে-বামে স্প্রে করা
  for(int i = angle - 15; i <= angle + 15; i++) { 
    waterServo.write(i); delay(20); 
  }
  for(int i = angle + 15; i >= angle - 15; i--) { 
    waterServo.write(i); delay(20); 
  }
  
  digitalWrite(PUMP, HIGH); // পাম্প বন্ধ
  waterServo.write(90);     // সার্ভো আবার সোজা
}

/*
#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial ESPSerial(10, 11); 
Servo waterServo;

// --- সেন্সর ও পিন কনফিগারেশন ---
#define LEFT_SENSOR 4
#define RIGHT_SENSOR 5

#define FLAME_L 6
#define FLAME_F 7
#define FLAME_R 8
#define SERVO_PIN 9
#define PUMP 12
#define BUZZER 3 // Active Buzzer Pin 3

char sysMode = 'M'; // ডিফল্ট মোড

void setup() {
  Serial.begin(9600);     
  ESPSerial.begin(9600);  

  pinMode(LEFT_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);
  
  // ফ্লেম সেন্সরগুলোতে INPUT_PULLUP দেওয়া হয়েছে (যাতে সেন্সর খোলা থাকলে বাজার না বাজে)
  pinMode(FLAME_L, INPUT_PULLUP);
  pinMode(FLAME_F, INPUT_PULLUP);
  pinMode(FLAME_R, INPUT_PULLUP);
  
  pinMode(PUMP, OUTPUT); 
  digitalWrite(PUMP, HIGH); // রিলে ডিফল্ট অফ
  
  pinMode(BUZZER, OUTPUT); 
  digitalWrite(BUZZER, LOW); // বাজার ডিফল্ট বন্ধ
  
  waterServo.attach(SERVO_PIN);
  waterServo.write(90); // সেন্টারে পজিশন
}

void loop() {
  // ১. ESP32 থেকে মোড রিসিভ করা
  if (ESPSerial.available() > 0) {
    char incoming = ESPSerial.read();
    if (incoming == 'M') {
      delay(10);
      if (ESPSerial.available() > 0) {
        sysMode = ESPSerial.read();
      }
    }
  }

  // --- ২. Line Follower Logic ---
  if (sysMode == 'A') {
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

  // --- ৩. Firefighter Logic ---
  else if (sysMode == 'F') {
    int fL = digitalRead(FLAME_L);
    int fF = digitalRead(FLAME_F);
    int fR = digitalRead(FLAME_R);

    if (fL == 0 || fF == 0 || fR == 0) {
      digitalWrite(BUZZER, HIGH); // Active Buzzer বাজানোর জন্য HIGH
      ESPSerial.print('S'); // চাকা থামানোর কমান্ড
      
      // কোন দিকের সেন্সর আগুন পেয়েছে সেই অনুযায়ী পানি ছিটানো
      if (fF == 0) extinguish(90);
      else if (fL == 0) extinguish(160);
      else if (fR == 0) extinguish(20);
      
    } else {
      digitalWrite(BUZZER, LOW);  // আগুন না থাকলে বাজার পুরোপুরি বন্ধ
      ESPSerial.print('S');
    }
    delay(100);
  }
}

// --- পানি ছিটানোর ফাংশন (Sweep Servo) ---
void extinguish(int angle) {
  waterServo.write(angle); 
  delay(500); 
  digitalWrite(PUMP, LOW); // পাম্প চালু
  
  // ডানে-বামে স্প্রে করা
  for(int i = angle - 15; i <= angle + 15; i++) { 
    waterServo.write(i); delay(20); 
  }
  for(int i = angle + 15; i >= angle - 15; i--) { 
    waterServo.write(i); delay(20); 
  }
  
  digitalWrite(PUMP, HIGH); // পাম্প বন্ধ
  waterServo.write(90);     // সার্ভো আবার সোজা
}*/
