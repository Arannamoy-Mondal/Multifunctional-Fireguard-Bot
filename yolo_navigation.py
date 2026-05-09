import cv2
import sys
import time
import requests
import numpy as np
from ultralytics import YOLO

model = YOLO('yolo11n.pt') 

ESP32_IP = "192.168.4.1" 
ESP32_CAPTURE_URL = f"http://{ESP32_IP}:80/capture"
ESP32_ACTION_URL = f"http://{ESP32_IP}/action?cmd=" 

print("Connecting to ESP32-CAM via Capture Stream...")

current_state = "S"
last_printed_state = ""
is_fullscreen = False 

yolo_auto_mode = False 
cycle_start_time = time.time()

# Smart Turn Variables
turn_delay_active = False
turn_start_time = 0
pending_turn_cmd = ""

def send_bot_command(cmd, description):
    global last_printed_state
    
    if cmd != last_printed_state:
        print(f">>> BOT COMMAND: {description} <<<")
        last_printed_state = cmd

        try:
            requests.get(ESP32_ACTION_URL + cmd, timeout=0.2)
        except requests.exceptions.RequestException:
            pass 

window_name = "ESP32-CAM YOLO Navigation"
cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)

print("\n--- CONTROLS ---")
print("Press 'A' to enable YOLO Auto Mode (moves autonomously)")
print("Press 'M' to enable Manual/Pause Mode (stops bot, use Web UI to drive)")
print("Press 'Q' to Quit")
print("----------------\n")

while True:
    try:
        img_resp = requests.get(ESP32_CAPTURE_URL, timeout=1.0)
        img_arr = np.array(bytearray(img_resp.content), dtype=np.uint8)
        frame = cv2.imdecode(img_arr, -1)
        
        if frame is None:
            continue
            
    except Exception as e:
        time.sleep(0.5)
        continue

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

    cv2.line(frame, (left_zone_end, 0), (left_zone_end, height), (255, 0, 0), 2)
    cv2.line(frame, (right_zone_start, 0), (right_zone_start, height), (255, 0, 0), 2)

    current_time = time.time()

    # --- MODE & CYCLE LOGIC ---
    if not yolo_auto_mode:
        cv2.putText(frame, "MODE: MANUAL/PAUSED (Press 'A' for Auto)", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
        
    else:
        # Auto Mode
        cv2.putText(frame, "MODE: YOLO AUTO (Press 'M' for Manual)", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        
        # 5s Move, 2s Brake Timer Logic
        cycle_elapsed = current_time - cycle_start_time
        
        if cycle_elapsed >= 7.0: 
            cycle_start_time = current_time 
            cycle_elapsed = 0.0

        if cycle_elapsed >= 5.0:
            send_bot_command('S', "CYCLE BRAKE (2s)")
            cv2.putText(frame, "CYCLE BRAKE (2s)...", (50, 80), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 165, 255), 3)
        else:
            
            if obstacle_in_path:
                if not turn_delay_active:
                    
                    send_bot_command('S', "OBSTACLE! BRAKING & SCANNING...")
                    cv2.putText(frame, "SCANNING L/R...", (50, 80), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 3)
                    turn_delay_active = True
                    turn_start_time = current_time
                    
                    
                    if left_side_objects == 0 and right_side_objects > 0:
                        pending_turn_cmd = 'L'
                    elif right_side_objects == 0 and left_side_objects > 0:
                        pending_turn_cmd = 'R'
                    else:
                        
                        pending_turn_cmd = 'L' if left_side_objects <= right_side_objects else 'R'
                
            
                elif current_time - turn_start_time >= 0.8:
                    if pending_turn_cmd == 'L':
                        send_bot_command('L', "LEFT IS CLEAR. TURNING LEFT")
                        cv2.putText(frame, "TURNING LEFT", (50, 80), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 165, 255), 3)
                    else:
                        send_bot_command('R', "RIGHT IS CLEAR. TURNING RIGHT")
                        cv2.putText(frame, "TURNING RIGHT", (50, 80), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 165, 255), 3)
            else:
                turn_delay_active = False
                send_bot_command('F', "PATH CLEAR. MOVE FORWARD")
                cv2.putText(frame, "FORWARD", (50, 80), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 3)

    cv2.imshow(window_name, frame)

    # --- Keyboard Listening ---
    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'): 
        send_bot_command('S', "STOPPING BOT AND EXITING")
        break
    elif key == ord('a'): # Enable Auto
        yolo_auto_mode = True
        cycle_start_time = time.time() 
        last_printed_state = "" 
        print("Switched to YOLO AUTO mode.")
        try: requests.get(f"http://{ESP32_IP}/mode?val=Y", timeout=0.2) 
        except: pass
    elif key == ord('m'): # Enable Manual/Pause
        yolo_auto_mode = False
        last_printed_state = "" 
        send_bot_command('S', "MANUAL MODE ACTIVATED")
        print("Switched to MANUAL mode.")
        try: requests.get(f"http://{ESP32_IP}/mode?val=M", timeout=0.2) 
        except: pass
    elif key == ord('f'): 
        is_fullscreen = not is_fullscreen
        if is_fullscreen:
            cv2.setWindowProperty(window_name, cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
        else:
            cv2.setWindowProperty(window_name, cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_NORMAL)

cv2.destroyAllWindows()
