import tkinter as tk
from tkinter import ttk
import threading
import time
from comm.audio import AudioTransmitter 

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

        # Start Recording Button
        self.record_button = ttk.Button(
            button_frame, text="Start Recording", 
            command=self.start_recording
        )
        self.record_button.pack(side="right", padx=10, pady=10)

        # Stop Recording Button
        self.stop_button = ttk.Button(
            button_frame, text="Stop Recording", 
            command=self.stop_recording
        )
        self.stop_button.pack(side="left", padx=10, pady=10)
        self.stop_button.config(state="disabled")

        # Countdown display
        self.countdown_label = tk.Label(
            main_frame, text="", font=("Arial", 36), fg="white", bg="black"
        )
        self.countdown_label.pack(expand=True)

        # Instance variables
        self.recording_thread = None
        self.is_recording = False
        self.audio_transmitter = AudioTransmitter([1])
        self.record_duration = 10 

    def start_recording(self):
        if self.is_recording:
            return  # Prevent multiple recordings at once

        self.is_recording = True
        self.record_button.config(state="disabled")  # Disable button during recording
        
        # Start countdown and recording in separate threads
        self.recording_thread = threading.Thread(target=self.record_audio)
        self.recording_thread.start()

        countdown_thread = threading.Thread(target=self.run_countdown)
        countdown_thread.start()

    def record_audio(self):
        """Start audio recording."""
        try:
            self.audio_transmitter.start_recording()
            time.sleep(self.record_duration)  # Simulate the recording duration
            self.audio_transmitter.stop_recording()
        finally:
            self.is_recording = False
            self.record_button.config(state="normal")  
            self.stop_button.config(state="disabled")

    def run_countdown(self):
        """Update the countdown label."""
        for i in range(self.record_duration, 0, -1):
            self.countdown_label.config(text=f"Recording... {i}s")
            time.sleep(1)

        self.countdown_label.config(text="Recording Complete!")

    def stop_recording(self):
        self.is_recording = False


