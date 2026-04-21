import cv2
import urllib.request
import numpy as np
import requests
from ultralytics import YOLO
import time

URL_STREAM = "http://192.168.4.1/capture"
URL_CMD = "http://192.168.4.1/action?cmd="

model = YOLO("yolo11n.pt") 


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
                
        
                if conf > 0.5 and (area / (frame_h * frame_w)) > 0.30:
                    object_in_front = True
                    cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), (0, 0, 255), 3)
                    cv2.putText(frame, "DANGER: OBSTACLE", (int(x1), int(y1)-10), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0,0,255), 2)

        if object_in_front and not is_waiting and not is_scanning:
            print("Obstacle Detected! Stopping for 10s...")
            send_cmd("O") 
            is_waiting = True
            last_detection_time = time.time()


        if is_waiting:
    
            if time.time() - last_detection_time > 11:
                print("10s passed. Now Scanning for empty path...")
                is_waiting = False
                is_scanning = True


        if is_scanning and not object_in_front:
            print("Path Clear! Resuming Normal Mode...")
            send_cmd("C") 
            is_scanning = False

        cv2.imshow("YOLO FireBot AI Vision", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            send_cmd("S")
            break
            
    except Exception as e:
        print(f"Error: {e}")
        time.sleep(1)

cv2.destroyAllWindows()
