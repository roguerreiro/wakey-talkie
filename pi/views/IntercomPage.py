import tkinter as tk
from tkinter import ttk
import threading
import RPi.GPIO as GPIO
from comm.audio import AudioRecorder
# from comm.rxtx import send_audio_file  # Assuming a function to send the file

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
        self.audio_recorder = AudioRecorder("real_time_recording.wav")
        self.recording_thread = None

        # Setup GPIO
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

        # Add interrupt for button
        GPIO.add_event_detect(BUTTON_PIN, GPIO.BOTH, callback=self.handle_button_event, bouncetime=200)

    def handle_button_event(self, channel):
        """Handle rising and falling edge events on the button."""
        if GPIO.input(BUTTON_PIN):  # Button pressed
            self.start_recording()
        else:  # Button released
            self.stop_recording()

    def start_recording(self):
        """Start recording when the button is pressed."""
        if self.recording_thread and self.recording_thread.is_alive():
            return  # Prevent multiple recordings at once

        self.status_label.config(text="Recording...")
        
        # Start recording and transmission in a separate thread
        self.recording_thread = threading.Thread(target=self.record_and_transmit)
        self.recording_thread.start()

    def record_and_transmit(self):
        """Record audio and transmit after recording finishes."""
        try:
            self.audio_recorder.record_and_save_audio()  # Record and save audio
            self.status_label.config(text="Recording Complete. Sending...")

            # Transmit the saved audio file
            # send_audio_file("real_time_recording.wav")
            self.status_label.config(text="Transmission Complete.")
        except Exception as e:
            self.status_label.config(text=f"Error: {e}")
        finally:
            self.status_label.config(text="Idle")

    def stop_recording(self):
        """Stop recording when the button is released."""
        # No action needed here, as the recording automatically stops after the set duration

    def cleanup(self):
        """Clean up GPIO and other resources."""
        GPIO.cleanup()
