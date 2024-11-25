import sounddevice as sd
import numpy as np
from scipy.io.wavfile import write
import queue
import threading
from comm.rxtx import setup, send_audio
import time
import wave

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
        self.audio_queue = queue.Queue(maxsize=10)
        self.stream = None  # Initialize stream as None
        self.mode = "transmit"
        self.running = True
        self.file = "/home/pi/wakey-talkie/audio/hitsdifferent8.wav"
        print(sd.query_devices(2))
        self.processing_thread = None

    # Callback function to process audio in real-time
    def audio_callback(self, indata, frames, time, status):
        if status:
            print(status)  # Handle any errors

        # Place audio data into the queue for processing
        try:
            self.audio_queue.put(indata.copy(), block=False)
        except queue.Full:
            print("Queue full! Dropping audio frame.")

    def process_audio_queue(self):
        """Thread function to process audio data from the queue."""
        while self.running:
            try:
                # Get audio data from the queue
                indata = self.audio_queue.get(block=True, timeout=1)
                
                # Process the data
                downsampled = indata[::SCALING_FACTOR] * 32
                audio_bytes = ((downsampled + 1) * 255 / 2).astype(np.uint8).tobytes()

                # Transmit or save based on the mode
                if self.mode == "transmit":
                    for i in range(0, len(audio_bytes), MAX_PACKET_SIZE):
                        packet = audio_bytes[i:i + MAX_PACKET_SIZE]
                        send_audio(packet)
                elif self.mode == "save":
                    print("Saving audio...")  # Replace with save logic if needed
            except queue.Empty:
                continue

    def start_processing_thread(self):
        """Start the background thread for audio processing."""
        self.running = True
        self.processing_thread = threading.Thread(target=self.process_audio_queue, daemon=True)
        self.processing_thread.start()

    def stop_processing_thread(self):
        """Stop the background thread for audio processing."""
        self.running = False
        if self.processing_thread and self.processing_thread.is_alive():
            self.processing_thread.join()

    def start_recording(self, type="transmit"):
        """Start recording audio."""
        self.mode = type
        try:
            self.audio_queue.queue.clear()  # Clear any previous data in the queue

            # Start the audio processing thread
            self.start_processing_thread()

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
        """Stop recording audio."""
        if self.stream:
            try:
                self.stream.stop()
                self.stream.close()
                print("Recording stopped.")
            except Exception as e:
                print(f"Error stopping recording: {e}")

        # Stop the processing thread
        self.stop_processing_thread()

    def read_and_send_audio(self):
        """
        Reads audio data from a WAV file in chunks of 32 samples and sends it
        with the correct timing for live playback.
        """
        # Open the WAV file
        with wave.open(self.file, 'rb') as wav_file:
            # Check file properties
            channels = wav_file.getnchannels()
            sample_width = wav_file.getsampwidth()
            frame_rate = wav_file.getframerate()

            if channels != 1:
                raise ValueError("Audio file must be mono.")
            if frame_rate != 16000:
                raise ValueError("Sampling rate must be 16 KHz.")

            print(f"Channels: {channels}, Sample Width: {sample_width}, Frame Rate: {frame_rate}")

            # Read audio data in chunks of 32 samples
            samples_per_chunk = 32
            chunk_period = samples_per_chunk / frame_rate  # Time in seconds to wait between chunks

            while True:
                frames = wav_file.readframes(samples_per_chunk)
                if not frames:
                    break  # End of file

                # Send the chunk
                send_audio(frames)  # Pass raw bytes to the sending function

                # Wait for the appropriate period
                time.sleep(chunk_period)

    def save_audio_from_queue(self):
        """Save audio from the queue into a WAV file."""
        audio_data = []
        while not self.audio_queue.empty():
            audio_data.append(self.audio_queue.get())

        if audio_data:
            audio_data = np.concatenate(audio_data, axis=0)
            write("real_time_recording.wav", RECORD_SAMPLE_RATE, audio_data)
            print("Recording saved as real_time_recording.wav")
        else:
            print("No audio data to save.")


# Example usage
if __name__ == "__main__":
    # Replace with actual peripheral IDs
    peripheral_ids = [1]

    setup()
    transmitter = AudioTransmitter(peripheral_ids)

    try:
        transmitter.start_recording(type="transmit")
        print("Recording started. Press Ctrl+C to stop.")
        while True:
            pass  # Keep the main thread alive
    except KeyboardInterrupt:
        print("Stopping recording...")
        transmitter.stop_recording()
