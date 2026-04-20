import cv2
import urllib.request
import numpy as np
import requests
from ultralytics import YOLO
import time

URL_STREAM = "http://192.168.4.1/capture"
URL_CMD = "http://192.168.4.1/action?cmd="

model = YOLO("yolo11n.pt") 
is_scanning = False

def send_cmd(cmd):
    try: requests.get(URL_CMD + cmd, timeout=1)
    except: pass

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
                # Bounding box area check kora (front obstacle bujhte)
                x1, y1, x2, y2 = box.xyxy[0]
                area = (x2 - x1) * (y2 - y1)
                conf = float(box.conf[0])
                
                if conf > 0.5 and (area / (frame_h * frame_w)) > 0.35:
                    object_in_front = True
                    cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), (0,0,255), 2)

        # Logic for Stop and Scan
        if object_in_front and not is_scanning:
            print("Obstacle! Sending Stop Signal...")
            send_cmd("O")
            is_scanning = True
            # ESP32 ekhn 10s thambe tarpor nijei ghurte thakbe
            
        elif not object_in_front and is_scanning:
            # Scanning mode-e thaka obosthay jodi rasta khali pay
            print("Path Clear! Resuming...")
            send_cmd("C")
            is_scanning = False

        cv2.imshow("YOLO FireBot Vision", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'): break
            
    except Exception as e:
        time.sleep(1)

cv2.destroyAllWindows()