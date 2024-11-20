import tkinter as tk
from tkinter import ttk
import threading
from comm.audio import AudioTransmitter  # Ensure this path is correct

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
        self.stop_button.config(state="disabled")  # Initially disabled

        # Status display
        self.status_label = tk.Label(
            main_frame, text="Idle", font=("Arial", 24), fg="white", bg="black"
        )
        self.status_label.pack(expand=True)

        # Instance variables
        self.recording_thread = None
        self.is_recording = False
        self.audio_transmitter = AudioTransmitter([1])

    def start_recording(self):
        """Start recording when the button is clicked."""
        if self.is_recording:
            return  # Prevent multiple recordings at once

        self.is_recording = True
        self.record_button.config(state="disabled")  # Disable Start button
        self.stop_button.config(state="normal")  # Enable Stop button
        self.status_label.config(text="Recording...")

        # Start recording in a separate thread
        self.recording_thread = threading.Thread(target=self.record_audio)
        self.recording_thread.start()

    def record_audio(self):
        """Start audio recording."""
        try:
            self.audio_transmitter.start_recording()
            while self.is_recording:
                pass  # Keep the recording active while the flag is True
            self.audio_transmitter.stop_recording()
        except Exception as e:
            self.status_label.config(text=f"Error: {e}")
        finally:
            self.stop_recording()  # Ensure cleanup

    def stop_recording(self):
        """Stop recording when the button is clicked."""
        if not self.is_recording:
            return  # Ignore if already stopped

        self.is_recording = False
        self.record_button.config(state="normal")  # Enable Start button
        self.stop_button.config(state="disabled")  # Disable Stop button
        self.status_label.config(text="Recording Stopped")
