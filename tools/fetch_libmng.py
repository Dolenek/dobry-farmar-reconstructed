#!/usr/bin/env python3
"""Fetch pinned LIBMNG source into ignored local storage, verifying Git blob hashes."""
import concurrent.futures
import argparse
import hashlib
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--verify-only", action="store_true", help="Check existing sources without network access")
    parser.add_argument("--source-dir", type=Path, default=ROOT / ".local" / "libmng-source")
    args = parser.parse_args()
    lock = json.loads((ROOT / "analysis" / "libmng-source.lock.json").read_text())
    output = args.source_dir
    output.mkdir(parents=True, exist_ok=True)

    def fetch(item):
        name = item["path"]
        if Path(name).name != name:
            raise ValueError("Only flat dependency source paths are allowed")
        target = output / name
        if not target.exists():
            if args.verify_only:
                raise ValueError(f"Missing pinned dependency file: {name}")
            url = f"https://raw.githubusercontent.com/LuaDist/libmng/{lock['commit']}/{name}"
            temporary = target.with_name(target.name + ".download")
            subprocess.run(["curl", "-sS", "-fL", "--retry", "2", "--connect-timeout", "10",
                            "--max-time", "60", url, "-o", str(temporary)], check=True)
        else:
            temporary = target
        data = temporary.read_bytes()
        actual = hashlib.sha1(b"blob " + str(len(data)).encode() + b"\0" + data).hexdigest()
        if actual != item["git_blob_sha1"]:
            raise ValueError(f"Dependency source hash mismatch: {name}")
        if temporary != target:
            temporary.replace(target)
        return name

    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
        names = list(pool.map(fetch, lock["files"]))
    unexpected = {p.name for p in output.iterdir() if p.suffix in (".c", ".h")} - set(names)
    if unexpected:
        raise ValueError(f"Unexpected dependency source files: {sorted(unexpected)}")
    print(f"Verified {len(names)} LIBMNG source files at {lock['commit']}")


if __name__ == "__main__":
    main()
