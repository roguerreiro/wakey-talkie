import tkinter as tk
import random
import time

class HomePage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent, bg="black")

        # Time display
        self.time_label = tk.Label(self, font=("Helvetica", 150), fg="white", bg="black")
        self.time_label.pack(expand=True, fill="both")
        self.update_time()

        # Create canvas for circular buttons
        self.canvas = tk.Canvas(self, bg="black", highlightthickness=0)
        self.canvas.pack(fill="both", expand=True)

        # Draw circular buttons
        self.create_circle_button(100, 100, 50, "AlarmPage", controller)
        self.create_circle_button(self.winfo_width() - 100, 100, 50, "IntercomPage", controller)

    def create_circle_button(self, x, y, r, target_page, controller):
        color = f"#{random.randint(0, 255):02x}{random.randint(0, 255):02x}{random.randint(0, 255):02x}"
        circle = self.canvas.create_oval(x - r, y - r, x + r, y + r, fill=color, outline=color)
        self.canvas.tag_bind(circle, "<Button-1>", lambda e: controller.show_frame(target_page))

    def update_time(self):
        current_time = time.strftime("%H:%M")
        self.time_label.config(text=current_time)
        self.after(1000, self.update_time)    