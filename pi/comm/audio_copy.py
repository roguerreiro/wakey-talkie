import sounddevice as sd
import numpy as np
from scipy.io.wavfile import write
import queue
import threading
from comm.rxtx import setup, send_audio
import time
import wave
import os

os.nice(-10)

# Recording parameters
RECORD_SAMPLE_RATE = 48000
SEND_SAMPLE_RATE = 16000
SCALING_FACTOR = 3
CHANNELS = 1
DURATION = 5  # Total duration for recording in seconds
CHUNK_SIZE = 96  # Number of frames per buffer for streaming
MAX_PACKET_SIZE = 32


class AudioTransmitter:
    def __init__(self, id_list):
        self.ids = id_list
        self.stream = None  # Initialize stream as None
        self.mode = "transmit"
        self.file = "/home/pi/wakey-talkie/audio/hitsdifferent8.wav"
        print(sd.query_devices(2))

    # Simplified callback function
    def audio_callback(self, indata, frames, time, status):
        if status:
            print(status)  # Handle any errors

        # Process and send the audio directly
        downsampled = indata[::SCALING_FACTOR] * 32
        audio_bytes = ((downsampled + 1) * 255 / 2).astype(np.uint8).tobytes()

        # Send packets directly
        for i in range(0, len(audio_bytes), MAX_PACKET_SIZE):
            packet = audio_bytes[i:i + MAX_PACKET_SIZE]
            send_audio(packet)

    def start_recording(self, type="transmit"):
        self.mode = type
        try:
            # Start the stream with the callback
            self.stream = sd.InputStream(
                samplerate=RECORD_SAMPLE_RATE,
                channels=CHANNELS,
                device=2,
                blocksize=CHUNK_SIZE,
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
            except Exception as e:
                print(f"Error stopping recording: {e}")
