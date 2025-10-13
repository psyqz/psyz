#!/usr/bin/env python3
import sys
import subprocess
import requests
import time
import struct
from PIL import Image
import os
import signal

def rgba5551_to_rgb888(data):
    """Convert RGBA5551 bytes to RGBA8888."""
    pixels = []
    for i in range(0, len(data), 2):
        val = struct.unpack_from("<H", data, i)[0]
        r = ((val >> 0) & 0x1F) << 3
        g = ((val >> 5) & 0x1F) << 3
        b = ((val >> 10) & 0x1F) << 3
        pixels.append((r, g, b))
    return pixels

def main():
    if len(sys.argv) != 3:
        print("Usage: psxgen.py <PCSX_REDUX_PATH> <EXE_PATH>")
        sys.exit(1)

    pcsx_path, exe_path = sys.argv[1], sys.argv[2]
    process = subprocess.Popen(
        [pcsx_path, "-loadexe", exe_path, "-run"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )
    time.sleep(5)

    try:
        resp = requests.get("http://localhost:6698/api/v1/gpu/vram/raw", timeout=5)
        resp.raise_for_status()
        raw = resp.content
    except Exception as e:
        print(f"Failed to fetch VRAM: {e}")
        process.terminate()
        process.wait()
        sys.exit(1)

    os.kill(process.pid, signal.SIGKILL)

    width, height = 1024, 512
    expected_size = width * height * 2
    if len(raw) < expected_size:
        print(f"ERROR: received only {len(raw)} bytes (expected {expected_size})")
        raw = raw.ljust(expected_size, b"\x00")
    elif len(raw) > expected_size:
        raw = raw[:expected_size]

    pixels = rgba5551_to_rgb888(raw)
    img = Image.new("RGB", (width, height))
    img.putdata(pixels)
    img.crop((0, 0, 256, 240)).save("output_fb1.png")
    img.crop((256, 0, 512, 240)).save("output_fb2.png")
    img.save("output_vram.png")

if __name__ == "__main__":
    main()
