"""Exercise the recovered wrapper with independently constructed PNG images."""
import os
import struct
import subprocess
import tempfile
import unittest
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DECODER = (ROOT / "build" / "windows" / "Release" / "jng-decode.exe"
                   if os.name == "nt" else ROOT / "build" / "jng-decode")
DECODER = Path(os.environ.get("DF_JNG_DECODER", DEFAULT_DECODER))


def chunk(kind, payload):
    return struct.pack(">I", len(payload)) + kind + payload + struct.pack(">I", zlib.crc32(kind + payload))


def make_png(width, height, rgb):
    rows = b"".join(b"\0" + rgb * width for _ in range(height))
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(rows)) + chunk(b"IEND", b""))


@unittest.skipUnless(DECODER.is_file(), "Build jng-decode before running decoder tests")
class DecoderTests(unittest.TestCase):
    def test_known_rgb_pixels_and_geometry(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "test.png"
            output = Path(directory) / "output.bgra"
            for width, height, rgb in [(1, 1, b"\xff\0\0"), (3, 2, b"\0\xff\0"), (7, 5, b"\0\0\xff")]:
                with self.subTest(width=width, height=height, rgb=rgb):
                    source.write_bytes(make_png(width, height, rgb))
                    result = subprocess.run([str(DECODER.resolve()), str(source), str(output)],
                                            check=True, capture_output=True, text=True, timeout=10)
                    self.assertEqual(tuple(map(int, result.stdout.split())), (width, height))
                    self.assertEqual(output.read_bytes(), bytes([rgb[2], rgb[1], rgb[0], 255]) * width * height)


if __name__ == "__main__":
    unittest.main()
