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

class AlarmPage(tk.Frame):
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

class MainApp(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)

        # self.attributes("-fullscreen", True)
        # self.bind("<Escape>", lambda event: self.attributes("-fullscreen", False))
        self.geometry("800x480")

        container = tk.Frame(self)
        container.pack(expand=True, fill="both")
        container.grid_rowconfigure(0, weight=1)
        container.grid_columnconfigure(0, weight=1)

        self.frames = {}
        # NOTE: add all page classes to this tuple
        for F in (HomePage, IntercomPage, AlarmPage):
            page_name = F.__name__
            frame = F(parent=container, controller=self)
            self.frames[page_name] = frame
            frame.grid(row=0, column=0, sticky="nsew")

        self.show_frame("HomePage")

    def show_frame(self, name):
        frame = self.frames[name]
        frame.tkraise()

if __name__ == "__main__":
    app = MainApp()
    app.mainloop()