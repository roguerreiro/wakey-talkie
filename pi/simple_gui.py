import tkinter as tk
from tkinter import ttk
import time

# Set main window attributes
root = tk.Tk()
root.title("Control Panel")
root.geometry("800x480")
root.configure(bg="black")  

# Create and configure a label for the time display
time_label = tk.Label(root, font=("Helvetica", 150), fg="white", bg="black")
time_label.pack(expand=True, fill="both")

# Function to update the time display
def update_time():
    current_time = time.strftime("%H:%M") 
    time_label.config(text=current_time)
    root.after(1000, update_time)

# Start the time update function
update_time()

root.mainloop()