import tkinter as tk
from views.HomePage import HomePage
from views.AlarmPage import AlarmPage
from views.IntercomPage import IntercomPage
import os

class MainApp(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)

        self.attributes("-fullscreen", True)
        self.bind("<Escape>", lambda event: self.attributes("-fullscreen", False))
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
    absolute_path = os.path.abspath(__file__)
    print(absolute_path)
    
    app = MainApp()
    app.mainloop()