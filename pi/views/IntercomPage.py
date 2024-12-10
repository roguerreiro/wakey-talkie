import tkinter as tk
from tkinter import ttk
from comm.Peripheral import Peripheral
from comm.audio import AudioRecorder
import threading

class IntercomPage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent, bg="white")

        # Instance variables
        self.controller = controller
        self.peripherals = {}
        self.checkbox_vars = {}
        self.audio_recorder = AudioRecorder()  # Audio recorder instance

        # Create a paned window
        self.pane = tk.PanedWindow(self, orient=tk.VERTICAL)
        self.pane.pack(fill="both", expand=True)

        # Frames for buttons and main display
        self.button_frame = tk.Frame(self.pane, bg="gray", height=80)
        self.main_frame = tk.Frame(self.pane, bg="white", height=400)
        self.pane.add(self.button_frame)
        self.pane.add(self.main_frame)

        # Back button
        back_button = ttk.Button(
            self.button_frame, text="Back",
            command=lambda: self.controller.show_frame("HomePage")
        )
        back_button.pack(side="left", padx=10, pady=10)

        # Refresh button
        refresh_button = ttk.Button(
            self.button_frame, text="Refresh",
            command=self.refresh_peripherals
        )
        refresh_button.pack(side="left", padx=10, pady=10)

        # Status label (created once)
        self.status_label = tk.Label(
            self.main_frame, text="Initializing...", font=("Arial", 16), bg="white", fg="red"
        )
        self.status_label.pack(expand=True, pady=20)


        # Initial refresh
        self.refresh_peripherals()

    def refresh_peripherals(self):
        """Refresh the list of available peripherals."""
        self.peripherals = Peripheral.get_available_devices()  # Fetch updated list
        self.checkbox_vars.clear()

        # Clear checkboxes and buttons from the main frame
        for widget in self.main_frame.winfo_children():
            if widget != self.status_label:
                widget.destroy()

        if not self.peripherals:
            # Update status label for no peripherals
            self.status_label.config(
                text="No available peripherals.", font=("Arial", 16), fg="red"
            )
            self.status_label.pack(expand=True, pady=20)
        else:
            # Hide the status label
            self.status_label.pack_forget()

            # Create checkboxes for each peripheral
            for id, peripheral in self.peripherals.items():
                var = tk.BooleanVar()
                self.checkbox_vars[id] = var
                checkbox = tk.Checkbutton(
                    self.main_frame, text=f"Peripheral {id}", variable=var, font=("Arial", 14), bg="white"
                )
                checkbox.pack(anchor="w", padx=20, pady=5)

            # Add send button
            send_button = ttk.Button(
                self.main_frame, text="Record and Send Audio", command=self.start_record_and_send
            )
            send_button.pack(pady=20)


    def start_record_and_send(self):
        """Start recording and then send the audio."""
        selected_peripherals = {
            id: self.peripherals[id]
            for id, var in self.checkbox_vars.items() if var.get()
        }

        if not selected_peripherals:
            self.status_label.config(text="No peripherals selected.", fg="red")
            return

        # Update UI to show recording status
        self.status_label.config(text="Press button to begin...", fg="blue")
        self.update_idletasks()

        self.audio_recorder.enable_recording()

        # Use a thread to avoid blocking the UI
        threading.Thread(target=self.record_and_send, args=(selected_peripherals,)).start()

    def record_and_send(self, selected_peripherals):
        """Record the audio and send it to selected peripherals."""
        try:
            # Record audio
            self.status_label.config(text="Recording...", fg="black")
            self.audio_recorder.record_and_save_audio()
            self.status_label.config(text="Recording complete. Sending audio...", fg="blue")
            self.update_idletasks()

            # Send audio file to each selected peripheral
            Peripheral.send_audio_file(selected_peripherals)

            self.status_label.config(text="Audio sent successfully.", fg="green")
        except Exception as e:
            self.status_label.config(text=f"Error: {e}", fg="red")

