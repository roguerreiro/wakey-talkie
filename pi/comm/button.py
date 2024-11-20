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

# def io_setup():
if __name__ == '__main__':   
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(BUTTON_GPIO, GPIO.IN, pull_up_down=GPIO.PUD_UP)

    GPIO.add_event_detect(BUTTON_GPIO, GPIO.FALLING, 
            callback=button_released_callback, bouncetime=100)
    
    GPIO.add_event_detect(BUTTON_GPIO, GPIO.RISING,
            callback=button_pressed_callback, bouncetime=100)
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.pause()