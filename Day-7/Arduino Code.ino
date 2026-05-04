// #include <SoftwareSerial.h>

// SoftwareSerial ESPSerial(10, 11); 

// #define LEFT_SENSOR 4
// #define RIGHT_SENSOR 5
// #define FLAME_L 6
// #define FLAME_F 7
// #define FLAME_R 8
// #define BUZZER 3

// char sysMode = 'M'; 

// // Non-blocking Variables
// unsigned long previousLineTime = 0;
// unsigned long previousFireSpamTime = 0;

// void setup() {
//   Serial.begin(9600);     
//   ESPSerial.begin(9600);  

//   pinMode(LED_BUILTIN, OUTPUT); 

//   pinMode(LEFT_SENSOR, INPUT);
//   pinMode(RIGHT_SENSOR, INPUT);
  
//   pinMode(FLAME_L, INPUT_PULLUP);
//   pinMode(FLAME_F, INPUT_PULLUP);
//   pinMode(FLAME_R, INPUT_PULLUP);
  
//   pinMode(BUZZER, OUTPUT); 
//   noTone(BUZZER); // বাজার বন্ধ
// }

// void loop() {
//   unsigned long currentMillis = millis();

//   // 1. ESP32 থেকে মোড রিসিভ করা (সঠিক পার্সিং লজিক)
//   while (ESPSerial.available() > 0) {
//     char c = ESPSerial.read();
    
//     // যদি 'M' পায়, তার মানে "M:A" বা "M:F" টাইপের মেসেজ আসছে
//     if (c == 'M') {
//       delay(10); // বাকি অক্ষরগুলো আসার জন্য সামান্য সময় দেওয়া
//       if (ESPSerial.available() >= 2) {
//         char colon = ESPSerial.read();
//         char modeChar = ESPSerial.read();
        
//         if (colon == ':') {
//           // মোড আপডেট করা
//           if (modeChar == 'A' || modeChar == 'F' || modeChar == 'M' || modeChar == 'E') {
//             sysMode = modeChar;
            
//             // ইমার্জেন্সি বা ম্যানুয়াল মোডে সব অ্যালার্ম অফ করে দেবে
//             if (sysMode == 'E' || sysMode == 'M') {
//               noTone(BUZZER);
//               digitalWrite(LED_BUILTIN, LOW);
//             }
//           }
//         }
//       }
//     }
//   }

//   // 2. LINE FOLLOWER LOGIC (ESP32 কে ওভারলোড না করে)
//   if (sysMode == 'A') {
//     if (currentMillis - previousLineTime >= 100) { // ২০ এর বদলে ১০০ms
//       int leftValue = digitalRead(LEFT_SENSOR);
//       int rightValue = digitalRead(RIGHT_SENSOR);

//       if (leftValue == 0 && rightValue == 0) ESPSerial.print('F');
//       else if (leftValue == 1 && rightValue == 0) ESPSerial.print('R');
//       else if (leftValue == 0 && rightValue == 1) ESPSerial.print('L');
//       else ESPSerial.print('S');
      
//       previousLineTime = currentMillis;
//     }
//   }

//   // // 3. FIREFIGHTER LOGIC (শুধুমাত্র Detection)
//   // else if (sysMode == 'F') {
//   //   int fL = digitalRead(FLAME_L);
//   //   int fF = digitalRead(FLAME_F);
//   //   int fR = digitalRead(FLAME_R);

//   //   // যদি সেন্সর আগুন পায়
//   //   if (fL == 0 || fF == 0 || fR == 0) {
//   //     digitalWrite(LED_BUILTIN, HIGH); // আর্ডুইনোর ভেতরের বাতি জ্বলবে
//   //     tone(BUZZER, 1000);              // বাজার বাজবে
      
//   //     ESPSerial.print('S');            // রোবটের চাকা ব্রেক করবে
//   //   } 
//   //   else {
//   //     // আগুন না থাকলে
//   //     digitalWrite(LED_BUILTIN, LOW);  // বাতি নিভে যাবে
//   //     noTone(BUZZER);                  // বাজার বন্ধ হবে
      
//   //     if (currentMillis - previousFireSpamTime >= 100) {
//   //       ESPSerial.print('S'); // রোবট দাঁড়িয়ে আগুন খুঁজবে
//   //       previousFireSpamTime = currentMillis;
//   //     }
//   //   }
//   // }


//   // // 3. FIREFIGHTER LOGIC (Patrol & Detection)
//   // else if (sysMode == 'F') {
//   //   int fL = digitalRead(FLAME_L);
//   //   int fF = digitalRead(FLAME_F);
//   //   int fR = digitalRead(FLAME_R);

//   //   // যদি সেন্সর আগুন পায় (Detection)
//   //   if (fL == 0 || fF == 0 || fR == 0) {
//   //     digitalWrite(LED_BUILTIN, HIGH); // আর্ডুইনোর ভেতরের বাতি জ্বলবে
//   //     tone(BUZZER, 1000);              // অ্যালার্ম বাজবে
      
//   //     ESPSerial.print('S');            // আগুন দেখলে সাথে সাথে ব্রেক কষবে!
//   //   } 
//   //   else {
//   //     // আগুন না থাকলে অ্যালার্ম বন্ধ থাকবে
//   //     digitalWrite(LED_BUILTIN, LOW);  
//   //     noTone(BUZZER);                  
      
//   //     // --- আগুন খোঁজার জন্য টহল (Patrol) দেবে ---
//   //     if (currentMillis - previousFireSpamTime >= 100) {
        
//   //       // লাইন ফলোয়ারের সেন্সর দিয়ে লাইন ট্র্যাক করে সামনে এগোবে
//   //       int leftValue = digitalRead(LEFT_SENSOR);
//   //       int rightValue = digitalRead(RIGHT_SENSOR);

//   //       if (leftValue == 0 && rightValue == 0) ESPSerial.print('F');       // সামনে
//   //       else if (leftValue == 1 && rightValue == 0) ESPSerial.print('R');  // ডানে
//   //       else if (leftValue == 0 && rightValue == 1) ESPSerial.print('L');  // বামে
//   //       else ESPSerial.print('S'); // লাইন হারিয়ে ফেললে দাঁড়াবে (বা ঘুরবে)
        
//   //       previousFireSpamTime = currentMillis;
//   //     }
//   //   }
//   // }

//   // 3. FIREFIGHTER LOGIC (Smart Patrol & Escape)
//   // else if (sysMode == 'F') {
//   //   // এই ভেরিয়েবলগুলো লুপের ভেতর মান ধরে রাখার জন্য static করা হয়েছে
//   //   static int patrolPhase = 0; 
//   //   static unsigned long phaseTimer = 0;
    
//   //   int fL = digitalRead(FLAME_L);
//   //   int fF = digitalRead(FLAME_F);
//   //   int fR = digitalRead(FLAME_R);

//   //   // --- ১. Fire Escape Logic (আগুন দেখলে) ---
//   //   if (fL == 0 || fF == 0 || fR == 0) {
//   //     digitalWrite(LED_BUILTIN, HIGH); 
//   //     tone(BUZZER, 1000);              
      
//   //     // আগুন দেখলে অনবরত এক জায়গায় ঘুরতে (Spin) থাকবে
//   //     if (currentMillis - previousFireSpamTime >= 100) {
//   //       ESPSerial.print('R'); // ডানে ঘুরবে (360 Search)
//   //       previousFireSpamTime = currentMillis;
//   //     }
      
//   //     // ঘুরতে ঘুরতে যেই মুহূর্তে সে আগুন-মুক্ত দিক পাবে, 
//   //     // তখন যেন সে আবার ২ সেকেন্ড সামনে ছুটে পালায় তাই ফেস 0 করে দেওয়া হলো
//   //     patrolPhase = 0; 
//   //     phaseTimer = currentMillis; 
//   //   } 
    
//   //   // --- ২. Patrol Logic (আগুন না থাকলে টহল দেওয়া) ---
//   //   else {
//   //     digitalWrite(LED_BUILTIN, LOW);  
//   //     noTone(BUZZER);                  
      
//   //     // Phase 0: ২ সেকেন্ড সামনে চলবে
//   //     if (patrolPhase == 0) {
//   //       if (currentMillis - previousFireSpamTime >= 100) {
//   //         ESPSerial.print('F'); // সামনে চলো
//   //         previousFireSpamTime = currentMillis;
//   //       }
//   //       if (currentMillis - phaseTimer >= 2000) { // ২ সেকেন্ড পর
//   //         patrolPhase = 1;
//   //         phaseTimer = currentMillis;
//   //       }
//   //     }
//   //     // Phase 1: একটু বামে ঘুরে চেক করবে (৪০০ মিলি-সেকেন্ড)
//   //     else if (patrolPhase == 1) {
//   //       if (currentMillis - previousFireSpamTime >= 100) {
//   //         ESPSerial.print('L'); // বামে ঘোরো
//   //         previousFireSpamTime = currentMillis;
//   //       }
//   //       if (currentMillis - phaseTimer >= 400) { 
//   //         patrolPhase = 2;
//   //         phaseTimer = currentMillis;
//   //       }
//   //     }
//   //     // Phase 2: ডানে ঘুরে চেক করবে (৮০০ মিলি-সেকেন্ড, মাঝখান পার হয়ে ডানে যাবে)
//   //     else if (patrolPhase == 2) {
//   //       if (currentMillis - previousFireSpamTime >= 100) {
//   //         ESPSerial.print('R'); // ডানে ঘোরো
//   //         previousFireSpamTime = currentMillis;
//   //       }
//   //       if (currentMillis - phaseTimer >= 800) {
//   //         patrolPhase = 3;
//   //         phaseTimer = currentMillis;
//   //       }
//   //     }
//   //     // Phase 3: আবার সোজা হওয়ার জন্য বামে ঘুরবে (৪০০ মিলি-সেকেন্ড)
//   //     else if (patrolPhase == 3) {
//   //       if (currentMillis - previousFireSpamTime >= 100) {
//   //         ESPSerial.print('L'); // আবার বামে ঘুরে সোজা হও
//   //         previousFireSpamTime = currentMillis;
//   //       }
//   //       if (currentMillis - phaseTimer >= 400) {
//   //         patrolPhase = 0; // স্ক্যান শেষ, আবার ২ সেকেন্ড সামনে চলবে
//   //         phaseTimer = currentMillis;
//   //       }
//   //     }
//   //   }
//   // }
//   // 5. FIREFIGHTER LOGIC (Locking Alarm & Radar Scan)
//   else if (sysMode == 'F') {
//     // লুপের বাইরে থাকা ভেরিয়েবলগুলোকে এখানেই static হিসেবে ডিক্লেয়ার করা হলো, 
//     // যাতে অন্য কোডের সাথে কোনো ঝামেলা না হয়।
//     static bool fireAlarmLocked = false;
//     static int patrolPhase = 0;
//     static unsigned long phaseTimer = 0;
//     static unsigned long previousRadarTime = 0;

//     // --- (A) আগুন দেখলে অ্যালার্ম লক করে দেওয়া ---
//     if (!fireAlarmLocked) { // যদি আগে থেকেই লক না থাকে, তাহলেই সেন্সর চেক করবে
//       int fL = digitalRead(FLAME_L);
//       int fF = digitalRead(FLAME_F);
//       int fR = digitalRead(FLAME_R);
      
//       if (fL == 0 || fF == 0 || fR == 0) {
//         fireAlarmLocked = true; // আগুন দেখলে লক করে দেবে
//       }
//     }

//     // --- (B) যদি অ্যালার্ম লক থাকে (Emergency না চাপা পর্যন্ত চলবে) ---
//     if (fireAlarmLocked) {
//       digitalWrite(LED_BUILTIN, HIGH); 
//       tone(BUZZER, 1000);              
      
//       // চাকা ব্রেক করে দাঁড়িয়ে থাকবে
//       if (currentMillis - previousFireSpamTime >= 100) {
//         ESPSerial.print('S'); 
//         previousFireSpamTime = currentMillis;
//       }
//     } 
    
//     // --- (C) যদি অ্যালার্ম লক না থাকে (আগুন খোঁজার রাডার মোড) ---
//     else {
//       digitalWrite(LED_BUILTIN, LOW);  
//       noTone(BUZZER);                  

//       // Phase 0: বামে স্ক্যান (৫ ডিগ্রি)
//       if (patrolPhase == 0) {
//         if (currentMillis - previousRadarTime >= 100) {
//           ESPSerial.print('L'); 
//           previousRadarTime = currentMillis;
//         }
//         if (currentMillis - phaseTimer >= 150) { 
//           patrolPhase = 1; phaseTimer = currentMillis; 
//         }
//       }
//       // Phase 1: ডানে স্ক্যান (১০ ডিগ্রি, অর্থাৎ বামের ৫ ডিগ্রি ব্যাক করে আরও ৫ ডিগ্রি ডানে)
//       else if (patrolPhase == 1) {
//         if (currentMillis - previousRadarTime >= 100) {
//           ESPSerial.print('R'); 
//           previousRadarTime = currentMillis;
//         }
//         if (currentMillis - phaseTimer >= 300) { 
//           patrolPhase = 2; phaseTimer = currentMillis; 
//         }
//       }
//       // Phase 2: সেন্টারে ফিরে আসা (বামে ৫ ডিগ্রি)
//       else if (patrolPhase == 2) {
//         if (currentMillis - previousRadarTime >= 100) {
//           ESPSerial.print('L'); 
//           previousRadarTime = currentMillis;
//         }
//         if (currentMillis - phaseTimer >= 150) { 
//           patrolPhase = 3; phaseTimer = currentMillis; 
//         }
//       }
//       // Phase 3: ৩৬০ ডিগ্রি বা বড় টার্ন নেওয়া (জায়গা পরিবর্তন)
//       else if (patrolPhase == 3) {
//         if (currentMillis - previousRadarTime >= 100) {
//           ESPSerial.print('R'); // ডানে বড় টার্ন
//           previousRadarTime = currentMillis;
//         }
//         if (currentMillis - phaseTimer >= 800) { 
//           patrolPhase = 0; phaseTimer = currentMillis; // আবার নতুন করে স্ক্যান শুরু
//         }
//       }
//     }
//   }
// }


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
  noTone(BUZZER); // বাজার বন্ধ
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. ESP32 থেকে মোড রিসিভ করা
  while (ESPSerial.available() > 0) {
    char c = ESPSerial.read();
    
    // যদি 'M' পায়, তার মানে "M:A" বা "M:F" টাইপের মেসেজ আসছে
    if (c == 'M') {
      delay(10); 
      if (ESPSerial.available() >= 2) {
        char colon = ESPSerial.read();
        char modeChar = ESPSerial.read();
        
        if (colon == ':') {
          // মোড আপডেট করা
          if (modeChar == 'A' || modeChar == 'F' || modeChar == 'M' || modeChar == 'E') {
            sysMode = modeChar;
            
            // ইমার্জেন্সি বা ম্যানুয়াল মোডে সব অ্যালার্ম অফ করে দেবে এবং লক খুলে দেবে
            if (sysMode == 'E' || sysMode == 'M') {
              fireAlarmLocked = false; // অ্যালার্ম রিসেট করা হলো
              noTone(BUZZER);
              digitalWrite(LED_BUILTIN, LOW);
            }
          }
        }
      }
    }
  }

  // 2. LINE FOLLOWER LOGIC (ESP32 কে ওভারলোড না করে)
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

  // 3. FIREFIGHTER LOGIC (Locking Alarm & Radar Scan)
  else if (sysMode == 'F') {
    static int patrolPhase = 0;
    static unsigned long phaseTimer = 0;
    static unsigned long previousRadarTime = 0;

    // --- (A) আগুন দেখলে অ্যালার্ম লক করে দেওয়া ---
    if (!fireAlarmLocked) { 
      int fL = digitalRead(FLAME_L);
      int fF = digitalRead(FLAME_F);
      int fR = digitalRead(FLAME_R);
      
      if (fL == 0 || fF == 0 || fR == 0) {
        fireAlarmLocked = true; // আগুন দেখলে লক করে দেবে
      }
    }

    // --- (B) যদি অ্যালার্ম লক থাকে (Emergency না চাপা পর্যন্ত চলবে) ---
    if (fireAlarmLocked) {
      digitalWrite(LED_BUILTIN, HIGH); 
      tone(BUZZER, 1000);              
      
      // চাকা ব্রেক করে দাঁড়িয়ে থাকবে
      if (currentMillis - previousFireSpamTime >= 100) {
        ESPSerial.print('S'); 
        previousFireSpamTime = currentMillis;
      }
    } 
    
    // --- (C) যদি অ্যালার্ম লক না থাকে (আগুন খোঁজার রাডার মোড) ---
    else {
      digitalWrite(LED_BUILTIN, LOW);  
      noTone(BUZZER);                  

      // Phase 0: সামনে চলা (Patrol Forward) - ৪ সেকেন্ড ধরে এগোবে
      if (patrolPhase == 0) {
        if (currentMillis - previousRadarTime >= 100) {
          ESPSerial.print('F'); // সামনে চলো
          previousRadarTime = currentMillis;
        }
        if (currentMillis - phaseTimer >= 4000) { // ৪ সেকেন্ড পর স্ক্যান শুরু করবে
          patrolPhase = 1; phaseTimer = currentMillis; 
        }
      }
      // Phase 1: বামে স্ক্যান (৫ ডিগ্রি - ১৫০ms)
      else if (patrolPhase == 1) {
        if (currentMillis - previousRadarTime >= 100) {
          ESPSerial.print('L'); 
          previousRadarTime = currentMillis;
        }
        if (currentMillis - phaseTimer >= 150) { 
          patrolPhase = 2; phaseTimer = currentMillis; 
        }
      }
      // Phase 2: ডানে স্ক্যান (১০ ডিগ্রি - ৩০০ms)
      else if (patrolPhase == 2) {
        if (currentMillis - previousRadarTime >= 100) {
          ESPSerial.print('R'); 
          previousRadarTime = currentMillis;
        }
        if (currentMillis - phaseTimer >= 300) { 
          patrolPhase = 3; phaseTimer = currentMillis; 
        }
      }
      // Phase 3: সেন্টারে ফিরে আসা (বামে ৫ ডিগ্রি - ১৫০ms)
      else if (patrolPhase == 3) {
        if (currentMillis - previousRadarTime >= 100) {
          ESPSerial.print('L'); 
          previousRadarTime = currentMillis;
        }
        if (currentMillis - phaseTimer >= 150) { 
          patrolPhase = 4; phaseTimer = currentMillis; 
        }
      }
      // Phase 4: ৩৬০ ডিগ্রি বা বড় টার্ন নেওয়া (৮০০ms)
      else if (patrolPhase == 4) {
        if (currentMillis - previousRadarTime >= 100) {
          ESPSerial.print('R'); // ডানে বড় টার্ন
          previousRadarTime = currentMillis;
        }
        if (currentMillis - phaseTimer >= 800) { 
          patrolPhase = 0; phaseTimer = currentMillis; // আবার Phase 0 (সামনে চলা) শুরু
        }
      }
    }
  }
}

