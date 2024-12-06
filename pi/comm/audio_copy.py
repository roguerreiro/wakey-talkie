import sounddevice as sd
import queue
import threading
import numpy as np
from comm.rxtx import send_audio  # Replace with your actual function

# Constants
RECORD_SAMPLE_RATE = 48000
SEND_SAMPLE_RATE = 16000
SCALING_FACTOR = 3
CHUNK_SIZE = 96
MAX_PACKET_SIZE = 32
RING_BUFFER_SIZE = 48000  # 1 second of audio at 48 kHz

class AudioTransmitter:
    def __init__(self, id_list):
        self.ids = id_list
        self.stream = None
        self.mode = "save"
        self.audio_queue = queue.Queue(maxsize=50)  # Queue for buffered audio
        self.ring_buffer = RingBuffer(size=RING_BUFFER_SIZE)  # Real-time backup
        self.running = False
        self.thread = None

    def audio_callback(self, indata, frames, time, status):
        if status:
            print("Audio callback status:", status)

        # Attempt to enqueue the raw audio data
        try:
            self.audio_queue.put_nowait(indata.copy())
        except queue.Full:
            print("Queue full! Writing to ring buffer.")
            self.ring_buffer.write(indata.copy())  # Store in ring buffer

    def processing_thread(self):
        while self.running:
            try:
                # Retrieve audio data from queue (fallback to ring buffer)
                audio_chunk = self.audio_queue.get(timeout=1)
            except queue.Empty:
                print("Queue empty! Reading from ring buffer.")
                audio_chunk = self.ring_buffer.read(CHUNK_SIZE)

            # Downsample
            downsampled = audio_chunk[::SCALING_FACTOR] * 32
            # Scale and encode
            audio_bytes = ((downsampled + 1) * 255 / 2).astype(np.uint8).tobytes()

            # Send packets
            for i in range(0, len(audio_bytes), MAX_PACKET_SIZE):
                packet = audio_bytes[i:i + MAX_PACKET_SIZE]
                send_audio(packet)

    def start_recording(self, type="transmit"):
        self.mode = type
        self.running = True
        if type == "transmit":
            self.thread = threading.Thread(target=self.processing_thread, daemon=True)
            self.thread.start()

            try:
                self.stream = sd.InputStream(
                    samplerate=RECORD_SAMPLE_RATE,
                    channels=1,
                    device=2,
                    blocksize=CHUNK_SIZE,
                    callback=self.audio_callback
                )
                self.stream.start()
            except Exception as e:
                print(f"Error starting recording: {e}")
                self.running = False
        elif type == "save":
            pass

    def stop_recording(self):
        self.running = False
        if self.stream:
            try:
                self.stream.stop()
                self.stream.close()
            except Exception as e:
                print(f"Error stopping recording: {e}")

        if self.thread and self.thread.is_alive():
            self.thread.join()
        print("Recording stopped.")

import numpy as np
import threading


class RingBuffer:
    def __init__(self, size):
        self.buffer = np.zeros(size, dtype=np.float32)  # Allocate buffer space
        self.head = 0  # Points to the next write position
        self.size = size
        self.lock = threading.Lock()  # Ensure thread safety

    def write(self, data):
        with self.lock:
            length = len(data)
            end = self.head + length
            if end > self.size:  # Wrap-around
                overflow = end - self.size
                self.buffer[self.head:] = data[:length - overflow]
                self.buffer[:overflow] = data[length - overflow:]
            else:
                self.buffer[self.head:end] = data
            self.head = (self.head + length) % self.size  # Update head position

    def read(self, length):
        with self.lock:
            start = (self.head - length) % self.size
            if start < 0:
                start += self.size
            return np.concatenate((self.buffer[start:], self.buffer[:start]))[-length:]
