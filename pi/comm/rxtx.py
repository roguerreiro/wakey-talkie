from pyrf24 import RF24, RF24_PA_LOW
import RPi.GPIO as GPIO
import time

PERIPHERAL_ADDRESS = 0xF0F0F0F0E1

# CE and CSN pins for nRF24L01+ on Raspberry Pi
radio = RF24(17, 0)

# Define hub address
hubAddress = 0xF0F0F0F0D2   # Address to receive responses from the peripheral

def setup():
    #Initialize GPIO pins
    GPIO.setmode(GPIO.BCM)
    
    # Initialize the radio
    radio.begin()
    radio.setPALevel(RF24_PA_LOW)  # Set power level to low for testing
    radio.setChannel(75)           # Ensure the same channel on both devices

def send_message(ids, message, acks=True):
    # Define the message to send
    radio.stopListening()  # Stop listening to transmit data
    radio.openWritingPipe(PERIPHERAL_ADDRESS)
    for id in ids:
        message = id + "|" + message
    success = radio.write(message.encode('utf-8'))  # Send message
    if success:
        print("Message sent successfully")
    else:
        print("Message sending failed")
    return success

def receive_message():        
    if radio.available():
        received_message = []
        received_message = radio.read(radio.getDynamicPayloadSize())
        return received_message
    
def set_alarm(id: int, time: str, sound: str=""):
    message = str(id) + "|" + time
    send_message([id], message)
