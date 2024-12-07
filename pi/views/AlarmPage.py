import tkinter as tk
from tkinter import ttk
from comm.rxtx import set_alarm
from comm.Peripheral import Peripheral


class AlarmPage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent, bg="white")

        self.parent = parent
        self.controller = controller

        self.peripherals = {}
        self.alarm_frames = {}
        self.main_frame = None

        self.init_ui()

    def init_ui(self):
        """Initializes the UI components."""
        pane = tk.PanedWindow(self, orient=tk.VERTICAL)
        pane.pack(fill="both", expand=True)

        # Top frame with buttons
        button_frame = tk.Frame(pane, bg="gray", height=80)
        pane.add(button_frame)

        # Main frame for peripherals
        self.main_frame = tk.Frame(pane, bg="white", height=400)
        pane.add(self.main_frame)

        # Back Button
        back_button = ttk.Button(
            button_frame, text="Back", command=lambda: self.controller.show_frame("HomePage")
        )
        back_button.pack(side="left", padx=5, pady=5)

        # Refresh Button
        refresh_button = ttk.Button(button_frame, text="Refresh", command=self.refresh_peripherals)
        refresh_button.pack(side="right", padx=5, pady=5)

        # Load peripherals initially
        self.refresh_peripherals()

    def refresh_peripherals(self):
        """Refreshes the list of peripherals and updates the UI."""
        # Clear existing widgets in the main frame
        for widget in self.main_frame.winfo_children():
            widget.destroy()
        self.alarm_frames.clear()

        # Fetch updated peripherals
        self.peripherals = Peripheral.get_available_devices()
        print(self.peripherals)

        if not self.peripherals:
            # Display message if no peripherals are found
            no_devices_label = tk.Label(
                self.main_frame,
                text="No available peripherals found.",
                font=("Helvetica", 16),
                bg="white",
                fg="red",
            )
            no_devices_label.pack(expand=True, pady=20)  # Using pack since no other widgets will be added
        else:
            # Create alarm frames for each peripheral
            for i, (id, peripheral) in enumerate(self.peripherals.items()):
                frame = self.create_alarm_frame(self.main_frame, id)
                frame.grid(row=i, column=0, pady=10, padx=10, sticky="ew")  # Use grid for peripherals
                self.alarm_frames[id] = frame


    def create_alarm_frame(self, parent, peripheral_id):
        """Creates a frame for configuring the alarm for a specific peripheral."""
        frame = tk.Frame(parent, bg="white", relief="groove", bd=2)

        tk.Label(frame, text=f"Peripheral {peripheral_id}", font=("Helvetica", 16), bg="white").pack()

        # Alarm Type Dropdown
        alarm_type_var = tk.StringVar(value="Type 1")
        alarm_type_menu = ttk.OptionMenu(frame, alarm_type_var, "Type 1", "Type 1", "Type 2", "Type 3")
        alarm_type_menu.pack(pady=5)

        # Hour and Minute Spinboxes
        hour_var = tk.StringVar(value="12")
        hour_spinbox = tk.Spinbox(
            frame, from_=1, to=12, wrap=True, textvariable=hour_var, font=("Helvetica", 16), width=2, justify="center", state="readonly"
        )
        hour_spinbox.pack(side="left", padx=5)

        colon_label = tk.Label(frame, text=":", font=("Helvetica", 16), bg="white")
        colon_label.pack(side="left")

        minute_var = tk.StringVar(value="00")
        minute_spinbox = tk.Spinbox(
            frame, from_=0, to=59, wrap=True, textvariable=minute_var, font=("Helvetica", 16), width=2, justify="center", state="readonly"
        )
        minute_spinbox.pack(side="left", padx=5)

        am_pm_var = tk.StringVar(value="AM")
        am_pm_menu = ttk.OptionMenu(frame, am_pm_var, "AM", "AM", "PM")
        am_pm_menu.config(width=2)
        am_pm_menu.pack(side="left", padx=5)

        # Submit Button
        submit_button = tk.Button(
            frame,
            text="Set Alarm",
            font=("Helvetica", 12),
            command=lambda: self.submit_time(peripheral_id, hour_var, minute_var, am_pm_var, alarm_type_var),
        )
        submit_button.pack(pady=10)

        # Store references to variables for later use
        frame.vars = {"hour": hour_var, "minute": minute_var, "am_pm": am_pm_var, "type": alarm_type_var}
        return frame

    def submit_time(self, peripheral_id, hour_var, minute_var, am_pm_var, alarm_type_var):
        """Handles submission of alarm settings for a specific peripheral."""
        hour = hour_var.get()
        minute = minute_var.get()
        am_pm = am_pm_var.get()
        alarm_type = alarm_type_var.get()

        print(f"Peripheral {peripheral_id} - Time Set: {hour}:{minute} {am_pm}, Type: {alarm_type}")

        self.peripherals[peripheral_id].set_alarm(hour, minute, am_pm)
