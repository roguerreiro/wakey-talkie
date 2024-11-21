from pyrf24 import RF24, RF24_PA_LOW
import RPi.GPIO as GPIO
import time
from comm.files import read_data, save_data

PERIPHERAL_ADDRESS = 0xF0F0F0F0E1
FILE_PATH = "~/wakey-talkie/pi/data.json"

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

def send_message(ids, message, encode=True):
    # Define the message to send
    radio.setAutoAck(True)
    radio.stopListening()  # Stop listening to transmit data
    radio.openWritingPipe(PERIPHERAL_ADDRESS)
    for id in ids:
        message = str(id) + "|" + str(message)
    if encode:
        message = message.encode('utf-8')
    success = radio.write(message)  # Send message
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
    
def send_audio(sample):
    radio.setAutoAck(False)
    radio.openWritingPipe(PERIPHERAL_ADDRESS)
    radio.write(sample)

def set_alarm(id, time):
    send_message([id], time, encode=False)

def get_available_peripherals():
    data = read_data(FILE_PATH)
    peripherals = data['peripherals']
    available = []
    for peripheral in peripherals:
        for _ in range(5):
            sent = send_message([peripheral['id']], "ping")
            if sent:
                available.append(peripheral)
                continue
    return available
