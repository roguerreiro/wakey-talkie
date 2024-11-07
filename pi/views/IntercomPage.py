import tkinter as tk
from tkinter import ttk

class IntercomPage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent, bg="white")
        
        pane = tk.PanedWindow(self, orient=tk.VERTICAL)
        pane.pack(fill="both", expand=True)
        button_frame = tk.Frame(pane, bg="gray", height=80)
        main_frame = tk.Frame(pane, bg="black", height=400)
        pane.add(button_frame)
        pane.add(main_frame)

        back_button = ttk.Button(button_frame, text="Back",
                            command=lambda: controller.show_frame("HomePage"))
        back_button.pack(expand=True, fill="both")