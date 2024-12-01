import tkinter as tk
import random
import time

class HomePage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent, bg="black")

        # Wrapper frame for better layout control
        wrapper_frame = tk.Frame(self, bg="black")
        wrapper_frame.grid(row=0, column=0, sticky="nsew")
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(0, weight=1)

        # Time display in the middle of the screen
        self.time_label = tk.Label(wrapper_frame, font=("Helvetica", 150), fg="white", bg="black")
        self.time_label.grid(row=1, column=0, pady=(20, 0), sticky="n")
        self.update_time()

        # Create canvas for circular buttons at the top
        self.canvas = tk.Canvas(wrapper_frame, bg="black", highlightthickness=0)
        self.canvas.grid(row=0, column=0, sticky="nsew")
        wrapper_frame.grid_rowconfigure(0, weight=1)
        wrapper_frame.grid_rowconfigure(1, weight=4)  # Time label should take more vertical space
        wrapper_frame.grid_columnconfigure(0, weight=1)

        # Wait for the window to be fully rendered before drawing buttons
        self.after(100, lambda: self.create_buttons(controller))

    def create_buttons(self, controller):
        self.update()  # Ensure accurate size measurements
        width = self.canvas.winfo_width()

        # Place buttons near the top corners
        self.create_circle_button(100, 50, 50, "Set Alarms", "AlarmPage", controller)
        self.create_circle_button(width - 100, 50, 50, "Intercom", "IntercomPage", controller)

    def create_circle_button(self, x, y, r, text, target_page, controller):
        color = f"#{random.randint(0, 255):02x}{random.randint(0, 255):02x}{random.randint(0, 255):02x}"
        circle = self.canvas.create_oval(x - r, y - r, x + r, y + r, fill=color, outline=color)
        
        # Add text inside the button
        label = self.canvas.create_text(x, y, text=text, fill="white", font=("Helvetica", 12, "bold"))

        # Bind click event to both circle and text
        self.canvas.tag_bind(circle, "<Button-1>", lambda e: controller.show_frame(target_page))
        self.canvas.tag_bind(label, "<Button-1>", lambda e: controller.show_frame(target_page))

    def update_time(self):
        current_time = time.strftime("%H:%M")
        self.time_label.config(text=current_time)
        self.after(1000, self.update_time)

