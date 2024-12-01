import tkinter as tk
import random
import time

class HomePage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent, bg="black")

        # Create a frame for the time label
        time_frame = tk.Frame(self, bg="black")
        time_frame.pack(fill="both", expand=True)

        # Time display
        self.time_label = tk.Label(time_frame, font=("Helvetica", 150), fg="white", bg="black")
        self.time_label.pack(expand=True, pady=(50, 0))  # Add padding to bring closer to center
        self.update_time()

        # Create canvas for circular buttons
        self.canvas = tk.Canvas(self, bg="black", highlightthickness=0)
        self.canvas.pack(fill="both", expand=True)

        # Delay button creation to allow for accurate positioning
        self.after(100, lambda: self.create_buttons(controller))

    def create_buttons(self, controller):
        self.update()  # Ensure window size is accurate
        width = self.winfo_width()

        # Place buttons in the top corners with slight margins
        self.create_circle_button(50, 50, 50, "Set Alarms", "AlarmPage", controller)  # Left button
        self.create_circle_button(width - 50, 50, 50, "Intercom", "IntercomPage", controller)  # Right button

    def create_circle_button(self, x, y, r, text, target_page, controller):
        color = f"#{random.randint(0, 255):02x}{random.randint(0, 255):02x}{random.randint(0, 255):02x}"
        circle = self.canvas.create_oval(x - r, y - r, x + r, y + r, fill=color, outline=color)

        # Add centered text inside the button
        label = self.canvas.create_text(x, y, text=text, fill="white", font=("Helvetica", 12, "bold"))

        # Bind click events to both circle and text
        self.canvas.tag_bind(circle, "<Button-1>", lambda e: controller.show_frame(target_page))
        self.canvas.tag_bind(label, "<Button-1>", lambda e: controller.show_frame(target_page))

    def update_time(self):
        current_time = time.strftime("%H:%M")
        self.time_label.config(text=current_time)
        self.after(1000, self.update_time)
