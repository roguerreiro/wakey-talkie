import sounddevice as sd
from scipy.io.wavfile import write
import threading
import RPi.GPIO as GPIO

# GPIO setup
BUTTON_PIN = 27  # GPIO pin connected to the button
GPIO.setmode(GPIO.BCM)
GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

# Recording parameters
INPUT_SAMPLE_RATE = 48000  # Microphone's sample rate
OUTPUT_SAMPLE_RATE = 16000  # Desired sample rate
CHANNELS = 1
MAXIMUM_DURATION = 10

class AudioRecorder:
    def __init__(self, filename="real_time_recording.wav"):
        self.filename = filename
        self.recording_thread = None
        self.is_recording = False
        self.recording_enabled = False

        # Add interrupt for button press
        GPIO.add_event_detect(BUTTON_PIN, GPIO.BOTH, callback=self.handle_button_event, bouncetime=200)

    def enable_recording(self):
        self.recording_enabled = True
    
    def handle_button_event(self, channel):
        """Handle button press and release events."""
        if not self.recording_enabled: return
        if GPIO.input(BUTTON_PIN):  # Button pressed
            self.start_recording()
        else:  # Button released
            self.stop_recording()

    def start_recording(self):
        """Start the recording in a separate thread."""
        if self.is_recording:
            return
        self.is_recording = True
        print("Recording started...")
        self.recording_thread = threading.Thread(target=self.record_audio)
        self.recording_thread.start()

    def record_and_save_audio(self):
        try:
            # Record at 48 kHz
            audio_data = sd.rec(
                int(MAXIMUM_DURATION * INPUT_SAMPLE_RATE),  # Maximum duration
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
        finally:
            self.is_recording = False

    def stop_recording(self):
        """Stop the recording."""
        if not self.is_recording:
            return
        print("Recording stopped.")
        # `sd.stop()` stops audio immediately, but data already recorded is saved
        sd.stop()

    def cleanup(self):
        """Clean up GPIO resources."""
        GPIO.cleanup()

# Example usage:
if __name__ == "__main__":
    recorder = AudioRecorder("real_time_recording.wav")
    try:
        print("Press the button to start/stop recording.")
        while True:
            pass  # Main loop
    except KeyboardInterrupt:
        print("Exiting...")
        recorder.cleanup()
