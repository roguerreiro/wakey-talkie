import time
import numpy as np

indata = np.random.random_sample(size=1024 * 3)
durations = []
for _ in range(100):  # Test 100 iterations
    start = time.time()
    downsampled = indata[0:len(indata):3]
    downsampled = downsampled * 32
    audio_bytes = ((downsampled + 1) * 255 / 2).astype(np.uint8).tobytes()
    end = time.time()
    durations.append(end - start)

print(f"Average processing time: {np.mean(durations):.6f} seconds")
