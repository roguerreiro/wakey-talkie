import sounddevice as sd
import numpy as np
from scipy.io.wavfile import write
import queue
from comm.rxtx import send_audio

# Recording parameters
SAMPLE_RATE = 44100
CHANNELS = 1
DURATION = 5  # Total duration for recording in seconds
CHUNK_SIZE = 1024  # Number of frames per buffer for streaming
MAX_PACKET_SIZE = 32

class AudioTransmitter:
    def __init__(self, id_list):
        self.ids = id_list
        self.audio_queue = queue.Queue()
        self.stream = None  # Initialize stream as None
        self.mode = "transmit"

    # Callback function to process audio in real-time
    def audio_callback(self, indata, frames, time, status):
        if status:
            print(status)  # Handle any errors

        audio_bytes = indata.tobytes()

        if self.mode == "transmit":
            for i in range(0, len(audio_bytes), MAX_PACKET_SIZE):
                packet = audio_bytes[i:i + MAX_PACKET_SIZE]
                send_audio(packet)
        elif self.mode == "save":
            self.audio_queue.put(indata.copy())

    # Start recording in real-time
    def start_recording(self, type="transmit"):
        self.mode = type
        try:
            self.audio_queue.queue.clear()  # Clear any previous data in the queue

            # Start the stream with the callback
            self.stream = sd.InputStream(
                samplerate=SAMPLE_RATE, 
                channels=CHANNELS, 
                callback=self.audio_callback
            )
            self.stream.start()                
        except Exception as e:
            print(f"Error starting recording: {e}")

    def stop_recording(self):
        if self.stream:
            try:
                self.stream.stop()
                self.stream.close()
                print("Recording stopped.")
                # Process the recorded chunks from the queue
                self.save_audio_from_queue()
            except Exception as e:
                print(f"Error stopping recording: {e}")

    def save_audio_from_queue(self):
        # Collect all chunks from the queue
        audio_data = []
        while not self.audio_queue.empty():
            audio_data.append(self.audio_queue.get())

        if audio_data:
            # Convert to a single numpy array and save as WAV file
            audio_data = np.concatenate(audio_data, axis=0)
            write("real_time_recording.wav", SAMPLE_RATE, audio_data)
            print("Recording saved as real_time_recording.wav")
        else:
            print("No audio data to save.")

# Example usage:
if __name__ == "__main__":
    # Replace with actual peripheral IDs
    peripheral_ids = [1]

    transmitter = AudioTransmitter(peripheral_ids)
    transmitter.start_recording(type="save")

    # Record for a fixed duration, then stop
    try:
        sd.sleep(DURATION * 1000)  # Sleep for recording duration (ms)
    finally:
        transmitter.stop_recording()
