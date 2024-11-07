import tkinter as tk
from tkinter import ttk
import time

class HomePage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent, bg="black")

        pane = tk.PanedWindow(self, orient=tk.VERTICAL)
        pane.pack(fill="both", expand=True)
        button_frame = tk.Frame(pane, bg="gray", height=80)
        time_frame = tk.Frame(pane, bg="black", height=400)
        pane.add(button_frame)
        pane.add(time_frame)

        button_frame.columnconfigure(0, weight=1)
        button_frame.columnconfigure(1, weight=1)
        alarm_button = ttk.Button(button_frame, text="Set Alarms",
                            command=lambda: controller.show_frame("AlarmPage"))
        alarm_button.grid(row=0, column=0, sticky="nsew")

        intercom_button = ttk.Button(button_frame, text="Send Message",
                            command=lambda: controller.show_frame("IntercomPage"))
        intercom_button.grid(row=0, column=1, sticky="nsew")

        self.time_label = tk.Label(time_frame, font=("Helvetica", 150), fg="white", bg="black")
        self.time_label.pack(expand=True, fill="both")
        self.update_time()
    
    def update_time(self):
        current_time = time.strftime("%H:%M") 
        self.time_label.config(text=current_time)
        self.after(1000, self.update_time)