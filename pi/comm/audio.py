import sounddevice as sd
from scipy.io.wavfile import write
import numpy as np

# Recording parameters
INPUT_SAMPLE_RATE = 48000  # Microphone's sample rate
OUTPUT_SAMPLE_RATE = 16000  # Desired sample rate
CHANNELS = 1
DURATION = 5  # Recording duration in seconds

class AudioRecorder:
    def __init__(self, filename="output.wav"):
        self.filename = filename

    def record_and_save_audio(self):
        print("Recording started...")
        try:
            # Record at 48 kHz
            audio_data = sd.rec(
                int(DURATION * INPUT_SAMPLE_RATE),
                samplerate=INPUT_SAMPLE_RATE,
                channels=CHANNELS,
                dtype='int16'
            )
            sd.wait()  # Wait for recording to finish

            # Downsample by taking every third sample
            downsampled_audio = audio_data[::3]

            # Save the downsampled audio
            write(self.filename, OUTPUT_SAMPLE_RATE, downsampled_audio)
            print(f"Recording saved as {self.filename}")
        except Exception as e:
            print(f"Error during recording: {e}")

# Example usage:
if __name__ == "__main__":
    recorder = AudioRecorder("real_time_recording.wav")
    recorder.record_and_save_audio()
