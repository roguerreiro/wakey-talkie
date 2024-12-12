import tkinter as tk
from tkinter import ttk
from comm.Peripheral import Peripheral
from comm.audio import AudioRecorder
import threading

AUDIO_PATH = "/home/pi/wakey-talkie/audio/recorded_audio.wav"

class IntercomPage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent, bg="white")

        # Instance variables
        self.controller = controller
        self.peripherals = {}
        self.checkbox_vars = {}
        self.audio_recorder = AudioRecorder(AUDIO_PATH)  # Audio recorder instance

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

        # Time selection widgets
        self.time_frame = tk.Frame(self.main_frame, bg="white")
        self.time_frame.pack(pady=10)

        self.create_time_selection()

        # Initial refresh
        self.refresh_peripherals()

    def create_time_selection(self):
        """Create time selection widgets for expiration time."""
        tk.Label(
            self.time_frame, text="Message Expiration Time:", font=("Helvetica", 14), bg="white"
        ).pack(pady=5)

        # Hour spinbox
        self.hour_var = tk.StringVar(value="12")
        hour_spinbox = tk.Spinbox(
            self.time_frame, from_=1, to=12, wrap=True, textvariable=self.hour_var,
            font=("Helvetica", 16), width=2, justify="center", state="readonly"
        )
        hour_spinbox.pack(side="left", padx=5)

        # Colon separator
        colon_label = tk.Label(self.time_frame, text=":", font=("Helvetica", 16), bg="white")
        colon_label.pack(side="left")

        # Minute spinbox
        self.minute_var = tk.StringVar(value="00")
        minute_spinbox = tk.Spinbox(
            self.time_frame, from_=0, to=59, wrap=True, textvariable=self.minute_var,
            font=("Helvetica", 16), width=2, justify="center", state="readonly"
        )
        minute_spinbox.pack(side="left", padx=5)

        # AM/PM dropdown
        self.am_pm_var = tk.StringVar(value="AM")
        am_pm_menu = ttk.OptionMenu(
            self.time_frame, self.am_pm_var, "AM", "AM", "PM"
        )
        am_pm_menu.config(width=3)
        am_pm_menu.pack(side="left", padx=5)

    def refresh_peripherals(self):
        """Refresh the list of available peripherals."""
        self.peripherals = Peripheral.get_available_devices()  # Fetch updated list
        self.checkbox_vars.clear()

        # Clear checkboxes and buttons from the main frame
        for widget in self.main_frame.winfo_children():
            if widget != self.status_label and widget != self.time_frame:
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
                    self.main_frame, text=f"Peripheral {id}", variable=var, font=("Arial", 18), bg="white"
                )
                checkbox.pack(anchor="w", padx=20, pady=10)

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

        # Get selected expiration time
        expiration_time = f"{self.hour_var.get()}:{self.minute_var.get()} {self.am_pm_var.get()}"
        print(f"Sent expiration time: {expiration_time}")
        expiration_time = {
            "hour": int(self.hour_var.get()),
            "minute": int(self.minute_var.get()),
            "AM": self.am_pm_var.get() == "AM"
        }

        # Update UI to show recording status
        self.status_label.config(text="Press button to begin...", fg="blue")
        self.update_idletasks()

        self.audio_recorder.enable_recording()

        # Disable the send button while recording/sending
        send_button = self.main_frame.winfo_children()[-1]
        send_button.config(state="disabled")

        # Use a thread to avoid blocking the UI
        threading.Thread(target=self.record_and_send, args=(selected_peripherals, expiration_time, send_button)).start()

    def record_and_send(self, selected_peripherals, expiration_time, send_button):
        """Record the audio and send it to selected peripherals."""
        try:
            # Record audio
            self.status_label.config(text="Recording...", fg="black")
            self.update_idletasks()
            self.audio_recorder.record_and_save_audio()

            # Update UI for sending status
            self.status_label.config(text="Recording complete. Sending audio...", fg="blue")
            self.update_idletasks()

            # Send audio file to each selected peripheral
            Peripheral.send_audio_file(selected_peripherals, expiration_time, AUDIO_PATH)

            # Update UI for success
            self.status_label.config(text="Audio sent successfully.", fg="green")
        except Exception as e:
            self.status_label.config(text=f"Error: {e}", fg="red")
        finally:
            # Re-enable the send button
            send_button.config(state="normal")
