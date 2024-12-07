import json
from comm.files import read_data, save_data
from comm.rxtx import send_message, Opcode
import os

FILE_PATH = "/home/pi/wakey-talkie/pi/data.json"

class Peripheral(object):
    def __init__(self, id):
        data = read_data(FILE_PATH)
        self.id = id
        self.address = data["peripherals"][id]["address"]

    def set_alarm(self, hour, minute, am_pm):
        hour_bits = int(hour) & 0b1111
        minute_bits = int(minute) & 0b00111111
        am = am_pm == "AM"
        message = (hour_bits << 12) | (minute_bits << 4) | (am << 3)
        print("{:16b}".format(message))

        # Split the 16-bit message into two bytes
        high_byte = (message >> 8) & 0xFF  # Extract the most significant 8 bits
        low_byte = message & 0xFF          # Extract the least significant 8 bits

        # Create a bytes object
        buffer = bytes([high_byte, low_byte])

        send_message(self.address, Opcode.SET_ALARM.value, buffer, tries=5)

    @staticmethod
    def get_available_devices():
        print(os.path.abspath(__file__))
        data = read_data(FILE_PATH)
        print(data)
        available ={}
        i = 0
        for peripheral in data["peripherals"]:
            success = send_message(peripheral["address"], Opcode.CONNECTION_CHECK.value, "", tries=3)
            if success:
                available[i] = Peripheral(i)
            i += 1
        return data