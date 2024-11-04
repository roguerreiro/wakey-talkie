import tkinter as tk
from tkinter import ttk

# Create the main window
root = tk.Tk()
root.title("Control Panel")
root.geometry("400x300")
root.configure(bg="#f2f2f2")  # Light gray background

# Create a frame for grouping controls
frame = ttk.Frame(root, padding=10)
frame.pack(fill="both", expand=True)

# Title label
title = ttk.Label(frame, text="Device Control Panel", font=("Helvetica", 16, "bold"))
title.grid(row=0, column=0, columnspan=2, pady=(0, 10))

style = ttk.Style()
style.configure("TButton", font=("Helvetica", 12), background="#4CAF50", foreground="white", padding=10)
style.map("TButton",
          foreground=[('pressed', 'white'), ('active', 'white')],
          background=[('pressed', '!disabled', '#3e8e41'), ('active', '#66bb6a')])

button = ttk.Button(root, text="Submit", style="TButton")
button.pack(pady=10)

# Button
toggle_button = ttk.Button(frame, text="Toggle LED", command=lambda: print("Toggled"))
toggle_button.grid(row=1, column=0, pady=5, padx=10, sticky="ew")

# Checkbox
check_var = tk.BooleanVar()
checkbox = ttk.Checkbutton(frame, text="Enable Feature", variable=check_var)
checkbox.grid(row=2, column=0, pady=5, padx=10, sticky="w")

# Status label
status_label = ttk.Label(frame, text="Status: OFF", font=("Helvetica", 12))
status_label.grid(row=3, column=0, columnspan=2, pady=10)

# Scale (Slider)
scale = ttk.Scale(frame, from_=0, to=100, orient="horizontal")
scale.grid(row=4, column=0, pady=5, padx=10, sticky="ew")

def create_gradient(canvas, width, height, color1, color2):
    r1, g1, b1 = root.winfo_rgb(color1)
    r2, g2, b2 = root.winfo_rgb(color2)
    r_ratio = (r2 - r1) / height
    g_ratio = (g2 - g1) / height
    b_ratio = (b2 - b1) / height

    for i in range(height):
        nr = int(r1 + (r_ratio * i))
        ng = int(g1 + (g_ratio * i))
        nb = int(b1 + (b_ratio * i))
        color = f"#{nr:04x}{ng:04x}{nb:04x}"
        canvas.create_line(0, i, width, i, fill=color)

canvas = tk.Canvas(root, width=400, height=300)
canvas.pack()
create_gradient(canvas, 400, 300, "#ff8c00", "#ffcc00")


# Function to update the status label
def update_status():
    if check_var.get():
        status_label.config(text="Status: ON")
    else:
        status_label.config(text="Status: OFF")

# Bind the checkbox to update status
checkbox.config(command=update_status)

# Run the main loop
root.mainloop()
