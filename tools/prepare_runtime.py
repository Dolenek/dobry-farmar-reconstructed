#!/usr/bin/env python3
"""Prepare isolated runtime data; do not copy original executable code."""
from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--original-dir", type=Path, default=ROOT.parent / "farmář" / "Dobrý farmář")
    p.add_argument("--build-dir", type=Path, help="Directory containing reconstructed binaries")
    p.add_argument("--output-dir", type=Path, default=ROOT / ".local" / "runtime")
    args = p.parse_args()
    source = args.original_dir.resolve(strict=True)
    output = args.output_dir.resolve()
    if source == output or source in output.parents or output in source.parents:
        p.error("Runtime and original directory must be separate")
    # Fresh directory avoids silently replacing saves or mixing original DLLs with new builds.
    if output.exists() and any(output.iterdir()):
        p.error("Output directory must be empty; choose a new directory")
    inventory = json.loads((ROOT / "analysis" / "inventory.json").read_text(encoding="utf-8"))
    expected = {f["path"]: f for f in inventory["files"]}
    data_paths = [source / "Data", source / "wdata"]
    for folder in data_paths:
        if not folder.is_dir():
            p.error(f"Required runtime data directory missing: {folder.name}")
        for path in folder.rglob("*"):
            if not path.is_file():
                continue
            relative = path.relative_to(source).as_posix()
            if relative not in expected:
                p.error(f"Unrecognized runtime data file: {relative}")
            digest = hashlib.sha256(path.read_bytes()).hexdigest()
            if digest != expected[relative]["sha256"]:
                p.error(f"Original data hash differs from reference: {relative}")
    binaries = ["Dobrý farmář.exe", "Squall.dll", "JNGLoad.dll", "Uninstall.exe"]
    if args.build_dir:
        build = args.build_dir.resolve(strict=True)
        for name in binaries:
            path = build / name
            if not path.is_file():
                p.error(f"Reconstructed binary missing: {name}")
            if hashlib.sha256(path.read_bytes()).hexdigest() == expected[name]["sha256"]:
                p.error(f"Build contains an unchanged original binary: {name}")
    output.mkdir(parents=True, exist_ok=True)
    for folder in data_paths:
        shutil.copytree(folder, output / folder.name)
    if args.build_dir:
        for name in binaries:
            shutil.copy2(args.build_dir / name, output / name)
    print(f"Prepared {'runtime' if args.build_dir else 'data only (game not reconstructed yet)'}: {output}")


if __name__ == "__main__":
    main()
