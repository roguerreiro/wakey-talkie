from pyrf24 import RF24, RF24_PA_LOW
import RPi.GPIO as GPIO
import time

# CE and CSN pins for nRF24L01+ on Raspberry Pi
radio = RF24(17, 0)

#GPIO pins for button input, send_led and rec_led
button_pin = 27
send_led = 22
rec_led = 18

# Define addresses
peripheral0Address = 0xF0F0F0F0E1  # Address to send to peripheral0
peripheral1Address = 0xF0F0F0F0C3 #Address to send to peripheral1
peripheral2Address = 0xF0F0F0F0B1 #Adddress to send to peripheral2
hubAddress = 0xF0F0F0F0D2   # Address to receive responses from the peripheral

def setup():
    #Initialize GPIO pins
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(button_pin, GPIO.IN, pull_up_down = GPIO.PUD_DOWN)
    GPIO.setup(send_led, GPIO.OUT)
    GPIO.setup(rec_led, GPIO.OUT)
    
    # Initialize the radio
    radio.begin()
    radio.setPALevel(RF24_PA_LOW)  # Set power level to low for testing
    radio.setChannel(75)           # Ensure the same channel on both devices)
    radio.openReadingPipe(0, peripheral0Address)
    radio.openReadingPipe(1, peripheral1Address)
    radio.openReadingPipe(2, peripheral2Address)
    radio.startListening()# Start listening immediately

def send_message(address):
    # Define the message to send
    radio.stopListening()  # Stop listening to transmit data
    radio.openWritingPipe(address)
    message = "Hello Peripheral"
    success = radio.write(message.encode('utf-8'))  # Send message
    if success:
        print("Message sent successfully")
        GPIO.output(send_led, GPIO.HIGH)
        time.sleep(0.1)
        GPIO.output(send_led, GPIO.LOW)
    else:
        print("Message sending failed")
        GPIO.output(send_led, GPIO.LOW)
        time.sleep(0.005)
        
    radio.openReadingPipe(0, peripheral0Address)
    radio.openReadingPipe(1, peripheral1Address)
    radio.openReadingPipe(2, peripheral2Address)
    radio.startListening()  # Return to listening mode for responses

def receive_message():
    pipe = radio.available_pipe()
        
    if radio.available():
        received_message = []
        received_message = radio.read(radio.getDynamicPayloadSize())
        GPIO.output(rec_led, GPIO.HIGH)
    
        if pipe[1] == 0:
            print("Received from peripheral0:", "".join(chr(i) for i in received_message))
            send_message(peripheral0Address)
            
        elif pipe[1] == 1:
            print("Received from peripheral1:", "".join(chr(i) for i in received_message))
            send_message(peripheral1Address)
        
        elif pipe[1] == 2:
            print("Received from peripheral2:", "".join(chr(i) for i in received_message))       
            send_message(peripheral2Address)
       
        time.sleep(0.1)
        GPIO.output(rec_led, GPIO.LOW)
        radio.startListening

if __name__ == "__main__":
    setup()
    while True:
        if GPIO.input(button_pin) == GPIO.HIGH:
            send_message(peripheral0Address)       # Send a message to the peripheral
        
        else:
            receive_message()    # Listen for a response
    time.sleep(0.01)
