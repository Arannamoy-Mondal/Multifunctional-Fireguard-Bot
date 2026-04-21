import cv2
from ultralytics import YOLO
import ollama
import threading
import time


STREAM_URL = "http://192.168.4.1:80/stream" 


model = YOLO("yolo11x.pt")


detected_objects = set()
current_direction = "Waiting for environment analysis..."

def get_ollama_direction():
  
    global current_direction, detected_objects
    
    while True:
        if detected_objects:
      
            objects_str = ", ".join(detected_objects)
            
     
            prompt = f"I am an autonomous system. I currently see: {objects_str}. Give me a concise 1-sentence logical direction or description of this scene."
            
            try:
             
                response = ollama.chat(model='gemma4:latest', messages=[
                    {'role': 'system', 'content': 'You are a helpful AI assistant for a robotics vision system. Be concise and practical.'},
                    {'role': 'user', 'content': prompt}
                ])
                current_direction = response['message']['content'].strip()
                print("\n[Ollama Direction]:", current_direction)
            except Exception as e:
                print(f"\n[Ollama Error]: {e}. Make sure Ollama service is running.")
        
        
        time.sleep(0) 


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


    results = model(frame, stream=True, verbose=False)
    
    current_frame_objects = set()
    
    for r in results:
        boxes = r.boxes
        for box in boxes:
        
            x1, y1, x2, y2 = map(int, box.xyxy[0])
            
     
            cls_id = int(box.cls[0])
            class_name = model.names[cls_id]
            current_frame_objects.add(class_name)
            
      
            cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
            cv2.putText(frame, class_name, (x1, y1 - 10), 
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