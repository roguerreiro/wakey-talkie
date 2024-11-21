import tkinter as tk
from tkinter import ttk
import threading
import RPi.GPIO as GPIO
from comm.audio_copy import AudioTransmitter  # Ensure the path is correct

# GPIO setup
BUTTON_PIN = 27  # GPIO pin connected to the button

class IntercomPage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent, bg="white")

        # Create a paned window
        pane = tk.PanedWindow(self, orient=tk.VERTICAL)
        pane.pack(fill="both", expand=True)

        # Frames for buttons and main display
        button_frame = tk.Frame(pane, bg="gray", height=80)
        main_frame = tk.Frame(pane, bg="black", height=400)
        pane.add(button_frame)
        pane.add(main_frame)

        # Back button
        back_button = ttk.Button(
            button_frame, text="Back",
            command=lambda: controller.show_frame("HomePage")
        )
        back_button.pack(side="left", padx=10, pady=10)

        # Status display
        self.status_label = tk.Label(
            main_frame, text="Idle", font=("Arial", 24), fg="white", bg="black"
        )
        self.status_label.pack(expand=True)

        # Instance variables
        self.is_recording = False
        self.audio_transmitter = AudioTransmitter([1])
        self.recording_thread = None

        # Setup GPIO
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

        # Add interrupt for button
        GPIO.add_event_detect(BUTTON_PIN, GPIO.RISING, callback=self.start_recording, bouncetime=200)
        GPIO.add_event_detect(BUTTON_PIN, GPIO.FALLING, callback=self.stop_recording, bouncetime=200)

    def handle_button_event(self, channel):
        """Handle rising and falling edge events on the button."""
        if GPIO.input(BUTTON_PIN):  # Button pressed
            self.start_recording()
        else:  # Button released
            self.stop_recording()

    def start_recording(self):
        """Start recording when the button is pressed."""
        if self.is_recording:
            return  # Prevent multiple recordings at once

        self.is_recording = True
        self.status_label.config(text="Recording...")
        
        # Start recording in a separate thread
        self.recording_thread = threading.Thread(target=self.record_audio)
        self.recording_thread.start()

    def record_audio(self):
        """Start audio recording."""
        try:
            self.audio_transmitter.start_recording()
            while self.is_recording:
                pass  # Keep recording active while the flag is True
            self.audio_transmitter.stop_recording()
        except Exception as e:
            self.status_label.config(text=f"Error: {e}")
        finally:
            if not self.is_recording:
                self.status_label.config(text="Recording Stopped")

    def stop_recording(self):
        """Stop recording when the button is released."""
        if not self.is_recording:
            return  # Ignore if already stopped

        self.is_recording = False
        self.status_label.config(text="Recording Stopped")

    def cleanup(self):
        """Clean up GPIO and other resources."""
        GPIO.cleanup()
