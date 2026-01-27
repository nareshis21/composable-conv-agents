import onnxruntime as ort
import sys
import os

model_path = r"C:\Users\nares\research\voice_agent_cpp\models\snac24_int2wav_static.onnx"

if not os.path.exists(model_path):
    print(f"Error: Model not found at {model_path}")
    sys.exit(1)

try:
    sess = ort.InferenceSession(model_path)
    print("Model loaded successfully.")
    print("\n--- Inputs ---")
    for i in sess.get_inputs():
        print(f"Name: {i.name}, Shape: {i.shape}, Type: {i.type}")
    
    print("\n--- Outputs ---")
    for o in sess.get_outputs():
        print(f"Name: {o.name}, Shape: {o.shape}, Type: {o.type}")

except Exception as e:
    print(f"Error inspecting model: {e}")
