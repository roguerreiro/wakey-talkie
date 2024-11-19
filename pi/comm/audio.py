import tkinter as tk
import sounddevice as sd
import numpy as np
from scipy.io.wavfile import write
import wave
import queue

# Recording parameters
SAMPLE_RATE = 44100
CHANNELS = 1
DURATION = 5  # Example total duration for recording in seconds
CHUNK_SIZE = 1024  # Number of frames per buffer for streaming

# Queue to hold audio chunks
audio_queue = queue.Queue()

# Callback function to process audio in real-time
def audio_callback(indata, frames, time, status):
    if status:
        print(status)  # print any errors
    audio_queue.put(indata.copy())  # Add the chunk to the queue
    print(indata)

# Start recording in real-time
def start_recording():
    audio_queue.queue.clear()  # Clear any previous data in the queue
    recording_button.config(state='disabled')
    stop_button.config(state='normal')
    
    # Start the stream with the callback
    global stream
    stream = sd.InputStream(samplerate=SAMPLE_RATE, channels=CHANNELS, callback=audio_callback)
    stream.start()

def stop_recording():
    # Stop the stream
    stream.stop()
    stream.close()
    recording_button.config(state='normal')
    stop_button.config(state='disabled')
    
    # Process the recorded chunks from the queue
    save_audio_from_queue()

def save_audio_from_queue():
    # Collect all chunks from the queue
    audio_data = []
    while not audio_queue.empty():
        audio_data.append(audio_queue.get())

    # Convert to a single numpy array and save as WAV file
    audio_data = np.concatenate(audio_data, axis=0)
    write("real_time_recording.wav", SAMPLE_RATE, audio_data)
    print("Recording saved as real_time_recording.wav")

# Tkinter GUI
root = tk.Tk()
root.title("Real-Time Audio Recorder")

# Start button
recording_button = tk.Button(root, text="Start Recording", command=start_recording)
recording_button.pack(pady=10)

# Stop button
stop_button = tk.Button(root, text="Stop Recording", command=stop_recording, state='disabled')
stop_button.pack(pady=10)

# Run GUI loop
root.mainloop()
