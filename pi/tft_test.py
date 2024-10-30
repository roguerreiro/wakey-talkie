import board
import busio
import digitalio
import ra8875

# Setup SPI bus and RA8875 display object
spi = busio.SPI(clock=board.SCLK, MOSI=board.MOSI, MISO=board.MISO)
cs = digitalio.DigitalInOut(board.CE0)
rst = digitalio.DigitalInOut(board.D22)  # Example GPIO for reset
display = adafruit_ra8875.RA8875(spi, cs, rst)

# Initialize display
display.initialize()
display.width = 800  # Set the width of your display
display.height = 480  # Set the height of your display

# Test drawing a rectangle
display.fill(0xFFFF)  # Fill screen with white
display.draw_rectangle(50, 50, 200, 100, 0x001F)  # Blue rectangle
