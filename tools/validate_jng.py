#!/usr/bin/env python3
"""Decode reference JNG assets, optionally comparing exact pixels with the original DLL."""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import subprocess
import tempfile
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def dimensions(data):
    if data[:8] != b"\x8bJNG\r\n\x1a\n":
        raise ValueError("Not a JNG image")
    offset = 8
    while offset + 12 <= len(data):
        size = struct.unpack_from(">I", data, offset)[0]
        if offset + 12 + size > len(data):
            raise ValueError("Truncated JNG chunk")
        if data[offset + 4:offset + 8] == b"JHDR":
            if size != 16:
                raise ValueError("Unexpected JHDR size")
            return struct.unpack_from(">II", data, offset + 8)
        offset += 12 + size
    raise ValueError("JHDR not found")


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--original-dir", type=Path, default=ROOT.parent / "farmář" / "Dobrý farmář")
    p.add_argument("--decoder", type=Path, default=ROOT / "build" / "jng-decode")
    p.add_argument("--output", type=Path, default=ROOT / ".local" / "jng-validation.json")
    p.add_argument("--reference-decoder", type=Path, help="Windows jng-reference-decode executable")
    p.add_argument("--reference-dll", type=Path, help="Original JNGLoad.dll, used only for comparison")
    args = p.parse_args()
    if bool(args.reference_decoder) != bool(args.reference_dll):
        p.error("--reference-decoder and --reference-dll must be supplied together")
    decoder = args.decoder.resolve(strict=True)
    reference = args.reference_decoder.resolve(strict=True) if args.reference_decoder else None
    reference_dll = args.reference_dll.resolve(strict=True) if args.reference_dll else None
    if reference_dll:
        inventory = json.loads((ROOT / "analysis" / "inventory.json").read_text(encoding="utf-8"))
        expected_dll = next(row["sha256"] for row in inventory["files"] if row["path"] == "JNGLoad.dll")
        if hashlib.sha256(reference_dll.read_bytes()).hexdigest() != expected_dll:
            p.error("Reference DLL does not match the inventoried original")
    work = ROOT / ".local"
    work.mkdir(exist_ok=True)
    rows = []
    failures = []
    compared = 0
    with tempfile.TemporaryDirectory(prefix="jng-validation-", dir=work) as temporary:
        image = Path(temporary) / "image.jng"
        pixels = Path(temporary) / "image.bgra"
        reference_pixels = Path(temporary) / "reference.bgra"
        for name in ("Data.pack", "cz.pack"):
            with zipfile.ZipFile(args.original_dir / "Data" / name) as archive:
                for entry in archive.infolist():
                    if not entry.filename.lower().endswith(".jng"):
                        continue
                    data = archive.read(entry)
                    row = dict(archive=name, entry=entry.filename,
                               input_sha256=hashlib.sha256(data).hexdigest())
                    try:
                        expected = dimensions(data)
                        image.write_bytes(data)
                        pixels.unlink(missing_ok=True)
                        result = subprocess.run([str(decoder), str(image), str(pixels)],
                                                capture_output=True, text=True, timeout=30)
                        if result.returncode:
                            raise RuntimeError(result.stderr.strip() or f"Exit {result.returncode}")
                        actual = tuple(int(x) for x in result.stdout.split())
                        if actual != expected:
                            raise ValueError(f"Decoded dimensions {actual} differ from JHDR {expected}")
                        output = pixels.read_bytes()
                        if len(output) != actual[0] * actual[1] * 4:
                            raise ValueError("Incorrect decoded buffer length")
                        row.update(width=actual[0], height=actual[1], output_size=len(output),
                                   bgra_sha256=hashlib.sha256(output).hexdigest(), status="decoded")
                        if reference:
                            reference_pixels.unlink(missing_ok=True)
                            result = subprocess.run([str(reference), str(reference_dll), str(image), str(reference_pixels)],
                                                    capture_output=True, text=True, timeout=30)
                            if result.returncode:
                                raise RuntimeError("Original DLL: " + (result.stderr.strip() or f"Exit {result.returncode}"))
                            reference_dimensions = tuple(int(x) for x in result.stdout.split())
                            original_pixels = reference_pixels.read_bytes()
                            row["reference_bgra_sha256"] = hashlib.sha256(original_pixels).hexdigest()
                            compared += 1
                            if reference_dimensions != actual or original_pixels != output:
                                raise ValueError("Decoded dimensions or BGRA pixels differ from original DLL")
                            row["status"] = "matches_original"
                    except (ValueError, RuntimeError, OSError, subprocess.TimeoutExpired) as error:
                        row.update(status="failed", error=str(error))
                        failures.append(row)
                    rows.append(row)
    report = dict(status="original_comparison" if reference else "diagnostic_only",
                  original_dll_compared=bool(reference) and bool(rows) and compared == len(rows) and not failures,
                  reference_requested=bool(reference), compared_images=compared,
                  images=len(rows), failed=len(failures), results=rows)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    suffix = f"compared {compared} with original DLL" if reference else "original DLL comparison not performed"
    print(f"Validated {len(rows) - len(failures)}/{len(rows)} JNG images; {suffix}")
    for failed in failures[:10]:
        print(f"  {failed['archive']}/{failed['entry']}: {failed['error']}")
    if failures or not rows:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
