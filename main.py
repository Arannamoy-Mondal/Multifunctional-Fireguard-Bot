import cv2
from ultralytics import YOLO
import ollama
import threading
import time
import numpy as np

STREAM_URL = "http://192.168.4.1:80/stream" 

model = YOLO("yolo11x.pt")

detected_objects = set()
current_direction = "Waiting for environment analysis..."


def detect_dominant_color(frame, x1, y1, x2, y2):
   
    roi = frame[max(0, y1):max(0, y2), max(0, x1):max(0, x2)]
    
    if roi.size == 0:
        return ""

  
    h, w = roi.shape[:2]
    cx, cy = w // 2, h // 2
    cw, ch = int(w * 0.3), int(h * 0.3)
    
    c_x1, c_y1 = max(0, cx - cw//2), max(0, cy - ch//2)
    c_x2, c_y2 = min(w, cx + cw//2), min(h, cy + ch//2)
    
    center_roi = roi[c_y1:c_y2, c_x1:c_x2]
    
    if center_roi.size == 0:
        center_roi = roi 
        

    hsv_roi = cv2.cvtColor(center_roi, cv2.COLOR_BGR2HSV)
    mean_color = cv2.mean(hsv_roi)
    h_val, s_val, v_val = mean_color[0], mean_color[1], mean_color[2]
    
  
    if v_val < 50: return "Black"
    if s_val < 50 and v_val > 200: return "White"
    if s_val < 50 and v_val <= 200: return "Gray"
    
    if h_val < 10 or h_val > 165: return "Red"
    elif 10 <= h_val < 25: return "Orange"
    elif 25 <= h_val < 35: return "Yellow"
    elif 35 <= h_val < 85: return "Green"
    elif 85 <= h_val < 130: return "Blue"
    elif 130 <= h_val < 165: return "Purple"
    
    return "Unknown-Color"


def get_ollama_direction():
    global current_direction, detected_objects
    last_seen_objects = set()
    
    while True:
        if not detected_objects:
            if last_seen_objects != set():
                current_direction = "Path is clear, move forward."
                print("\n[System Direction]:", current_direction)
                last_seen_objects = set() 
                
        elif detected_objects != last_seen_objects:
            objects_str = ", ".join(detected_objects)
            prompt = f"I am an autonomous fireguard system. I currently see: {objects_str}. Give me a concise 1-sentence logical direction or description of this scene."
            
            try:
                response = ollama.chat(model='gemma4:latest', messages=[
                    {'role': 'system', 'content': 'You are a helpful AI assistant for a robotics vision system. Be concise and practical.'},
                    {'role': 'user', 'content': prompt}
                ])
                current_direction = response['message']['content'].strip()
                print("\n[Ollama Direction]:", current_direction)
                
                last_seen_objects = detected_objects.copy()
            except Exception as e:
                print(f"\n[Ollama Error]: {e}. Make sure Ollama service is running.")
        
        time.sleep(0.5)

def yolo():
    global detected_objects, current_direction
    threading.Thread(target=get_ollama_direction, daemon=True).start()

    cap = cv2.VideoCapture(STREAM_URL)

    if not cap.isOpened():
        print("Error: Could not connect to the stream. Check Wi-Fi and URL.")
        exit()

    print("Successfully connected to ESP32-CAM. Press 'q' to quit.")
    cv2.namedWindow("ESP32-CAM Vision & AI Direction", cv2.WINDOW_NORMAL)
    cv2.resizeWindow("ESP32-CAM Vision & AI Direction", 800, 600)
    
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Stream interrupted!")
            break

        current_frame_objects = set()

        results = model(frame, stream=True, verbose=False)
        
        for r in results:
            boxes = r.boxes
            for box in boxes:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                
                cls_id = int(box.cls[0])
                class_name = model.names[cls_id]
                
    
                color_name = detect_dominant_color(frame, x1, y1, x2, y2)
                
      
                full_object_name = f"{color_name} {class_name}".strip()
                
                current_frame_objects.add(full_object_name)
                
       
                cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
                cv2.putText(frame, full_object_name, (x1, y1 - 10), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        detected_objects = current_frame_objects

        display_text = current_direction if len(current_direction) < 60 else current_direction[:57] + "..."
        cv2.rectangle(frame, (0, 0), (frame.shape[1], 40), (0, 0, 0), -1) 
        cv2.putText(frame, display_text, (10, 25), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 255, 255), 2)

        cv2.imshow("ESP32-CAM Vision & AI Direction", frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

def main():
    print("Hello from multifunctional-fireguard-bot!")
    yolo()

if __name__ == "__main__":
    main()