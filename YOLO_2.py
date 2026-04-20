import cv2
import urllib.request
import numpy as np
import requests
from ultralytics import YOLO
import time

URL_STREAM = "http://192.168.4.1/capture"
URL_CMD = "http://192.168.4.1/action?cmd="

model = YOLO("yolo11n.pt") 

# অবস্থা ট্র্যাক করার জন্য ভ্যারিয়েবল
is_waiting = False
is_scanning = False
last_detection_time = 0

def send_cmd(cmd):
    try:
        requests.get(URL_CMD + cmd, timeout=1)
        print(f"Command Sent: {cmd}")
    except:
        print("Failed to connect to ESP32")

while True:
    try:
        img_resp = urllib.request.urlopen(URL_STREAM)
        imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
        frame = cv2.imdecode(imgnp, -1)
        
        results = model(frame, verbose=False)
        object_in_front = False
        
        frame_h, frame_w, _ = frame.shape
        
        for r in results:
            for box in r.boxes:
                x1, y1, x2, y2 = box.xyxy[0]
                area = (x2 - x1) * (y2 - y1)
                conf = float(box.conf[0])
                
                # যদি বাধা খুব কাছে থাকে (ফ্রেমের ৩০% এর বেশি দখল করে)
                if conf > 0.5 and (area / (frame_h * frame_w)) > 0.30:
                    object_in_front = True
                    cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), (0, 0, 255), 3)
                    cv2.putText(frame, "DANGER: OBSTACLE", (int(x1), int(y1)-10), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0,0,255), 2)

        # --- মূল কন্ট্রোল লজিক ---

        # ১. যদি প্রথমবার বাধা পাওয়া যায়
        if object_in_front and not is_waiting and not is_scanning:
            print("Obstacle Detected! Stopping for 10s...")
            send_cmd("O") # ESP32 কে 'O' পাঠালে সে নিজে থেকেই ১০ সেকেন্ড থামবে
            is_waiting = True
            last_detection_time = time.time()

        # ২. ১০ সেকেন্ড ওয়েটিং পিরিয়ড শেষ হওয়ার পর স্ক্যানিং মুড একটিভ করা
        if is_waiting:
            # চেক করবে ১০ সেকেন্ড পার হয়েছে কি না
            if time.time() - last_detection_time > 11: # ১২ সেকেন্ড ধরা নিরাপদ (১০ সে থামবে + ২ সে বাফার)
                print("10s passed. Now Scanning for empty path...")
                is_waiting = False
                is_scanning = True

        # ৩. স্ক্যানিং মুডে যখন রাস্তা খালি পাবে
        if is_scanning and not object_in_front:
            print("Path Clear! Resuming Normal Mode...")
            send_cmd("C") # ESP32 কে 'C' পাঠালে সে ঘোরা বন্ধ করে দিবে
            is_scanning = False

        cv2.imshow("YOLO FireBot AI Vision", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            send_cmd("S") # 'q' চাপলে রোবট থামবে
            break
            
    except Exception as e:
        print(f"Error: {e}")
        time.sleep(1)

cv2.destroyAllWindows()
