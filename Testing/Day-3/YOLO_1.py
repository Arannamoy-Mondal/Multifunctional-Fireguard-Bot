import cv2
import sys
from ultralytics import YOLO


model = YOLO('yolo11n.pt') 

ESP32_URL = "http://192.168.x.x:81/stream" 

print("Connecting to ESP32-CAM...")
cap = cv2.VideoCapture(ESP32_URL)

if not cap.isOpened():
    print("No ESP32-CAM URL. USB Camera (Fallback) is starting")
    cap = cv2.VideoCapture(0)
    

    if not cap.isOpened():
        print("ERROR: No USB Camera. This programme is closing.")
        sys.exit() 

current_state = "FORWARD"
last_printed_state = ""
is_fullscreen = False 

def print_instruction(instruction):

    global last_printed_state
    if instruction != last_printed_state:
        print(f">>> BOT COMMAND: {instruction} <<<")
        last_printed_state = instruction

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

    # --- INSTANT LOGIC CONTROLLER ---
    if obstacle_in_path:

        if left_side_objects <= right_side_objects:
            current_state = "INSTANT STOP -> MOVE LEFT"
        else:
            current_state = "INSTANT STOP -> MOVE RIGHT"
        
        print_instruction(current_state)
        cv2.putText(frame, current_state, (50, 50), 
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 3)

    else:

        current_state = "FORWARD"
        print_instruction("PATH CLEAR. MOVE FORWARD.")
        cv2.putText(frame, "FORWARD", (50, 50), 
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 3)


    cv2.line(frame, (left_zone_end, 0), (left_zone_end, height), (255, 0, 0), 2)
    cv2.line(frame, (right_zone_start, 0), (right_zone_start, height), (255, 0, 0), 2)

    cv2.imshow(window_name, frame)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'): 
        break
    elif key == ord('f'): 
        is_fullscreen = not is_fullscreen
        if is_fullscreen:
            cv2.setWindowProperty(window_name, cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
        else:
            cv2.setWindowProperty(window_name, cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_NORMAL)

cap.release()
cv2.destroyAllWindows()