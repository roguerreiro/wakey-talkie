import tkinter as tk
from tkinter import ttk
from comm.rxtx import set_alarm

class AlarmPage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent, bg="white")
        
        pane = tk.PanedWindow(self, orient=tk.VERTICAL)
        pane.pack(fill="both", expand=True)
        button_frame = tk.Frame(pane, bg="gray", height=80)
        main_frame = tk.Frame(pane, bg="white", height=400)
        pane.add(button_frame)
        pane.add(main_frame)

        back_button = ttk.Button(button_frame, text="Back",
                            command=lambda: controller.show_frame("HomePage"))
        back_button.pack(expand=True, fill="both")

        # Hour spinbox (1-12)
        self.hour_var = tk.StringVar(value="12")
        hour_spinbox = tk.Spinbox(main_frame, from_=1, to=12, wrap=True, textvariable=self.hour_var,
                                  font=("Helvetica", 20), width=2, justify="center", state="readonly")
        hour_spinbox.grid(row=1, column=0, padx=5)

        # Separator for hours and minutes
        colon_label = tk.Label(main_frame, text=":", font=("Helvetica", 20))
        colon_label.grid(row=1, column=1)

        self.minute_var = tk.StringVar(value="00")
        minute_spinbox = tk.Spinbox(main_frame, from_=0, to=59, wrap=True, textvariable=self.minute_var,
                                    font=("Helvetica", 20), width=2, justify="center", state="readonly",
                                    format="%02.0f")
        minute_spinbox.grid(row=1, column=2, padx=5)

        self.am_pm_var = tk.StringVar(value="AM")
        am_pm_menu = ttk.OptionMenu(main_frame, self.am_pm_var, "AM", "AM", "PM")
        am_pm_menu.config(width=2)
        am_pm_menu.grid(row=1, column=3, padx=5)

        # Button to submit the time
        submit_button = tk.Button(main_frame, text="Submit Time", font=("Helvetica", 16), command=self.submit_time)
        submit_button.grid(row=2, column=0, columnspan=4, pady=20)

    def submit_time(self):
        # Retrieve input values
        hour = self.hour_var.get()
        minute = self.minute_var.get()
        am_pm = self.am_pm_var.get()
        print(f"Time Set: {hour}:{minute} {am_pm}")

        hour_bits = int(hour) & 0b0111
        minute_bits = int(minute) & 0b00111111
        am = am_pm == "AM"
        message = (hour_bits << 12) | (minute_bits << 4) | (am << 3)
        print("{:16b}".format(message))


        # message = f"{hour}:{minute}{am_pm}"
        set_alarm(1, message)
