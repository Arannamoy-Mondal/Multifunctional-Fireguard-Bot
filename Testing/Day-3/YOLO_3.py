import cv2
import sys
import requests
from ultralytics import YOLO

model = YOLO('yolo11n.pt') 

ESP32_IP = "192.168.4.1" # Replace with actual IP
ESP32_STREAM_URL = f"http://{ESP32_IP}:81/stream" 
ESP32_ACTION_URL = f"http://{ESP32_IP}/action?cmd=" 

print("Connecting to ESP32-CAM...")
cap = cv2.VideoCapture(ESP32_STREAM_URL)

if not cap.isOpened():
    print("No ESP32-CAM URL. USB Camera (Fallback) is starting")
    cap = cv2.VideoCapture(0)
    
    if not cap.isOpened():
        print("ERROR: No USB Camera. This program is closing.")
        sys.exit() 

current_state = "S"
last_printed_state = ""
is_fullscreen = False 

def send_bot_command(cmd, description):
    global last_printed_state
    
    if cmd != last_printed_state:
        print(f">>> BOT COMMAND: {description} <<<")
        last_printed_state = cmd

    # Always send command to keep the ESP32 Fail-safe alive
    try:
        requests.get(ESP32_ACTION_URL + cmd, timeout=0.1)
    except:
        pass

window_name = "ESP32-CAM YOLO Navigation"
cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)

while True:
    ret, frame = cap.read()
    if not ret:
        print("Frame can not read. Please check camera.")
        break

    height, width, _ = frame.shape
    
    left_zone_end = width // 3
    right_zone_start = (width // 3) * 2

    results = model(frame, conf=0.4, stream=True, verbose=False)
    
    obstacle_in_path = False
    left_side_objects = 0
    right_side_objects = 0

    for r in results:
        for box in r.boxes:
            x1, y1, x2, y2 = map(int, box.xyxy[0])
            conf = box.conf[0]
            cls = int(box.cls[0])
            class_name = model.names[cls] 
            
            center_x = (x1 + x2) // 2
            
            if left_zone_end < center_x < right_zone_start:
                obstacle_in_path = True
                box_color = (0, 0, 255) 
            else:
                box_color = (0, 255, 0)
            
            cv2.rectangle(frame, (x1, y1), (x2, y2), box_color, 2)
            cv2.putText(frame, f"{class_name} {conf:.2f}", (x1, y1 - 10), 
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, box_color, 2)

            if center_x < width // 2:
                left_side_objects += 1
            else:
                right_side_objects += 1

    # --- LOGIC CONTROLLER ---
    if obstacle_in_path:
        if left_side_objects <= right_side_objects:
            send_bot_command('L', "INSTANT STOP ->  MOVE LEFT")
            cv2.putText(frame, "MOVE LEFT", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 3)
        else:
            send_bot_command('R', "INSTANT STOP ->  MOVE RIGHT")
            cv2.putText(frame, "MOVE RIGHT", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 3)
    else:
        send_bot_command('F', "PATH CLEAR.  MOVE FORWARD")
        cv2.putText(frame, "FORWARD", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 3)

    cv2.line(frame, (left_zone_end, 0), (left_zone_end, height), (255, 0, 0), 2)
    cv2.line(frame, (right_zone_start, 0), (right_zone_start, height), (255, 0, 0), 2)

    cv2.imshow(window_name, frame)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'): 
        send_bot_command('S', "STOPPING BOT AND EXITING")
        break
    elif key == ord('f'): 
        is_fullscreen = not is_fullscreen
        if is_fullscreen:
            cv2.setWindowProperty(window_name, cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
        else:
            cv2.setWindowProperty(window_name, cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_NORMAL)

cap.release()
cv2.destroyAllWindows()