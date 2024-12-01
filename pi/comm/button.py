import signal
import sys
import RPi.GPIO as GPIO

BUTTON_GPIO = 16

def signal_handler(sig, frame):
    GPIO.cleanup()
    sys.exit(0)

def button_pressed_callback(channel):
    print("Button pressed!")

def button_released_callback(channel):
    print("Button released!")

class Button(object):
    def __init__(self, pin, rising_callback, falling_callback, bounce=100):
        self.pin = pin
        self.rising = rising_callback
        self.falling = falling_callback

        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        GPIO.add_event_detect(self.pin, GPIO.both, self.handle_press, bouncetime=bounce)

    def handle_press(self):
        if (GPIO.input(self.pin)):
            self.rising()
        else:
            self.falling()

# def io_setup():
if __name__ == '__main__':   
    button = Button(BUTTON_GPIO, button_pressed_callback, button_released_callback)
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.pause()