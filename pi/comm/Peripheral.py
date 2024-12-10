from comm.files import read_data, save_data
from comm.rxtx import setup, send_message, Opcode
from comm.audio import AudioRecorder
import wave

FILE_PATH = "/home/pi/wakey-talkie/pi/data.json"
AUDIO_PATH = "/home/pi/wakey-talkie/pi/real_time_recording.wav"

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

        success = send_message(self.address, Opcode.SET_ALARM.value, buffer, tries=5)
        if success:
            data = read_data(FILE_PATH)
            data["peripherals"][self.id]["alarm"] = f"{hour}:{minute}{'AM' if am_pm else 'PM'}"
            save_data(data, FILE_PATH)

    def send_audio(self, samples):
        send_message(self.address, Opcode.AUDIO_CHUNK.value, samples, tries=3)

    @staticmethod
    def get_available_devices():
        data = read_data(FILE_PATH)
        available ={}
        i = 0
        for peripheral in data["peripherals"]:
            success = send_message(peripheral["address"], Opcode.CONNECTION_CHECK.value, "".encode('utf-8'), tries=3)
            if success:
                available[i] = Peripheral(i)
            i += 1
        return available
    
    @staticmethod
    def send_audio_file(peripherals:dict, filename=AUDIO_PATH, chunk_size=31):
        peripherals_copy = peripherals.copy()
        try:
            with wave.open(filename, 'rb') as wav_file:
                n_channels = wav_file.getnchannels()
                sample_width = wav_file.getsampwidth()
                framerate = wav_file.getframerate()
                n_frames = wav_file.getnframes()

                print(f"Sending file: {filename}")
                print(f"Channels: {n_channels}, Sample width: {sample_width} bytes, Frame rate: {framerate}, Frames: {n_frames}")

                for id,peripheral in peripherals_copy.items():
                    success = send_message(peripheral.address, Opcode.CONNECTION_CHECK.value, "".encode('utf-8'), tries=5)
                    if not success: 
                        print(f"Failed to connect to id {id}")
                        del peripherals_copy[id]
                if len(peripherals_copy) == 0:
                    print("No peripherals available to send")
                    return

                while True:
                    frames = wav_file.readframes(31)
                    if not frames:
                        break
                    for id,peripheral in peripherals_copy.items():
                        success = send_message(peripheral.address, Opcode.AUDIO_CHUNK.value, frames, tries=1)
                        if not success: print(f"failed to send chunk to peripheral {id}")


                for id, peripheral in peripherals_copy.items():
                    success = send_message(peripheral.address, Opcode.AUDIO_FINISHED.value, "".encode('utf-8'), tries=5)
                
        
        except Exception as e:
            print(f"Error sending audio file: {e}")

if __name__ == "__main__":
    setup()
    filename = "test.wav"
    peripherals = {0:Peripheral(0)}
    recorder = AudioRecorder(filename)
    print("Starting recording...")
    recorder.record_and_save_audio()
    Peripheral.send_audio_file(peripherals, filename)
