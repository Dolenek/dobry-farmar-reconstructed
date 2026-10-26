#!/usr/bin/env python3
"""Fetch and verify original JPEG 6a, JPEG 6b / zlib 1.2.3 source into local-only storage."""
import argparse
import hashlib
import json
import subprocess
import tarfile
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--verify-only", action="store_true")
    args = parser.parse_args()
    lock = json.loads((ROOT / "analysis" / "codecs.lock.json").read_text())
    cache = ROOT / ".local" / "codec-archives"
    cache.mkdir(parents=True, exist_ok=True)
    for dependency in lock["dependencies"]:
        source = ROOT / ".local" / dependency["name"]
        if not args.verify_only:
            archive_path = cache / dependency["archive"]
            if not archive_path.exists():
                subprocess.run(["curl", "-sS", "-fL", "--retry", "2", "--connect-timeout", "10",
                                "--max-time", "60", dependency["url"], "-o", str(archive_path)], check=True)
            if hashlib.sha256(archive_path.read_bytes()).hexdigest() != dependency["sha256"]:
                raise ValueError(f"Archive hash mismatch: {dependency['archive']}")
            source.mkdir(parents=True, exist_ok=True)
            with tarfile.open(archive_path, "r:gz") as archive:
                for item in dependency["files"]:
                    relative = PurePosixPath(item["path"])
                    if relative.is_absolute() or ".." in relative.parts or len(relative.parts) != 1:
                        raise ValueError("Expected flat dependency source paths")
                    target = source / item["path"]
                    if target.exists():
                        continue  # Verification below refuses modified local sources.
                    member = archive.getmember(dependency["archive_root"] + "/" + item["path"])
                    if not member.isfile():
                        raise ValueError("Dependency archive entry is not a regular file")
                    data = archive.extractfile(member).read()
                    if hashlib.sha256(data).hexdigest() != item["sha256"]:
                        raise ValueError(f"Archived source hash mismatch: {item['path']}")
                    target.write_bytes(data)
        for item in dependency["files"]:
            path = source / item["path"]
            if hashlib.sha256(path.read_bytes()).hexdigest() != item["sha256"]:
                raise ValueError(f"Modified dependency source: {path}")
        print(f"Verified {dependency['name']}: {len(dependency['files'])} source files")


if __name__ == "__main__":
    main()
