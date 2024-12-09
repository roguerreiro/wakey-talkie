from pyrf24 import RF24, RF24_PA_LOW
import RPi.GPIO as GPIO
import time
from comm.files import read_data, save_data
import wave
from enum import Enum
PERIPHERAL_ADDRESS = 0xF0F0F0F0E1
FILE_PATH = "~/wakey-talkie/pi/data.json"

class Opcode(Enum):
    CONNECTION_CHECK = 0
    SET_ALARM = 1
    SET_EXPIRATION = 2
    AUDIO_INCOMING = 3
    AUDIO_FINISHED = 4

# CE and CSN pins for nRF24L01+ on Raspberry Pi
radio = RF24(17, 0)

# Define hub address
hubAddress = 0xF0F0F0F0D2   # Address to receive responses from the peripheral

def setup():
    #Initialize GPIO pins
    GPIO.setmode(GPIO.BCM)
    
    # Initialize the radio
    radio.begin()
    radio.setAutoAck(True)
    radio.setPALevel(RF24_PA_LOW)  # Set power level to low for testing
    radio.setChannel(75)           # Ensure the same channel on both devices
    print(radio.isChipConnected())

def send_message(address, opcode, message, tries=1):
    if len(message) > 31:
        print("Message too long. Must be 31 bytes or fewer.")
        return False

    payload = bytearray(32)
    payload[0] = opcode
    payload[1:1 + len(message)] = message.encode('utf-8')

    # Pad the message if it's shorter than 30 bytes
    if len(message) < 31:
        payload[1 + len(message):] = b'\x00' * (31 - len(message))

    # Send the payload
    radio.stopListening()
    radio.openWritingPipe(address)
    success = False
    for _ in range(tries):
        success = radio.write(payload)
        if success: break

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
    
def send_audio(samples):
    radio.openWritingPipe(PERIPHERAL_ADDRESS)
    radio.write(samples)

def send_audio_file(ids, filename, chunk_size=32):
    try:
        with wave.open(filename, 'rb') as wav_file:
            n_channels = wav_file.getnchannels()
            sample_width = wav_file.getsampwidth()
            framerate = wav_file.getframerate()
            n_frames = wav_file.getnframes()

            print(f"Sending file: {filename}")
            print(f"Channels: {n_channels}, Sample width: {sample_width} bytes, Frame rate: {framerate}, Frames: {n_frames}")

            for id in ids:
                success = send_message([id], Opcode.AUDIO_INCOMING, "", tries=5)
                if not success: 
                    print(f"Failed to send to id {id}")
                    return

            while True:
                frames = wav_file.readframes(chunk_size)
                if not frames:
                    break
                send_audio(frames)

            for id in ids:
                send_message([id], Opcode.AUDIO_FINISHED, "", tries=5)

    except Exception as e:
        print(f"Error sending audio file: {e}")

def set_alarm(id, time):
    send_message([id], Opcode.SET_ALARM, time, tries=5)

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