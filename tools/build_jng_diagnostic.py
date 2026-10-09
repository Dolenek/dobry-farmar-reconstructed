#!/usr/bin/env python3
"""Build the Linux diagnostic directly from the three pinned source dependencies."""
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFINES = ["MNG_SUPPORT_FULL", "MNG_NO_OPEN_CLOSE_STREAM", "MNG_INTERNAL_MEMMNGMT"]


def main():
    if os.name == "nt":
        raise SystemExit("This diagnostic is for Linux; use the MSVC CMake target on Windows")
    vendor = ROOT / ".local" / "libmng-source"
    lock = json.loads((ROOT / "analysis" / "libmng-source.lock.json").read_text())
    for item in lock["files"]:
        data = (vendor / item["path"]).read_bytes()
        if hashlib.sha1(b"blob " + str(len(data)).encode() + b"\0" + data).hexdigest() != item["git_blob_sha1"]:
            raise SystemExit(f"Modified dependency source: {item['path']}")
    cc, cxx = shutil.which("gcc"), shutil.which("g++")
    if not cc or not cxx:
        raise SystemExit("gcc and g++ are required")
    build = ROOT / "build" / "jng-diagnostic"
    build.mkdir(parents=True, exist_ok=True)
    codecs = json.loads((ROOT / "analysis" / "codecs.lock.json").read_text())
    codec_sources = []
    codec_include = []
    for dependency in codecs["dependencies"]:
        source_dir = ROOT / ".local" / dependency["name"]
        for item in dependency["files"]:
            if hashlib.sha256((source_dir / item["path"]).read_bytes()).hexdigest() != item["sha256"]:
                raise SystemExit(f"Modified codec source: {item['path']}")
        codec_include.append("-I" + str(source_dir))
        codec_sources.extend(source_dir / name for name in dependency["library_sources"])
    shutil.copyfile(ROOT / ".local" / "jpeg-6b" / "jconfig.vc", build / "jconfig.h")
    flags = ["-O2", "-fPIC", *["-D" + d for d in DEFINES], "-I" + str(vendor),
             *codec_include, "-I" + str(build), "-DHAVE_UNISTD_H"]
    objects = []
    for source in [*sorted(vendor.glob("*.c")), *codec_sources]:
        obj = build / (source.stem + ".o")
        subprocess.run([cc, "-std=gnu99", *flags, "-c", str(source), "-o", str(obj)], check=True)
        objects.append(str(obj))
    binary = ROOT / "build" / "jng-decode"
    subprocess.run([cxx, "-std=c++17", "-Wall", "-Wextra", "-Wpedantic", *flags,
                    "-I" + str(ROOT / "src" / "jngload"),
                    str(ROOT / "src" / "jngload" / "JNGLoad.cpp"), str(ROOT / "tools" / "jng_decode.cpp"),
                    *objects, "-lm", "-o", str(binary)], check=True)
    report = dict(kind="linux_diagnostic", libmng_commit=lock["commit"], compile_definitions=DEFINES,
                  binary_sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),
                  source_codecs=[d["name"] for d in codecs["dependencies"]],
                  original_libmng_version_confirmed=False, original_runtime_compared=False)
    (build / "build-info.json").write_text(json.dumps(report, indent=2) + "\n")
    print(f"Built decoder diagnostic: {binary}")


if __name__ == "__main__":
    main()
