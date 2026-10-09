#!/usr/bin/env python3
"""Generate verifiable metadata; never execute originals or extract game assets."""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import zipfile
from collections import Counter
from pathlib import Path
from pe import PE

ROOT = Path(__file__).resolve().parents[1]


def inspect(original: Path):
    files = []
    for path in sorted(original.rglob("*")):
        if not path.is_file() or any(p.endswith(".rep") for p in path.parts):
            continue
        if path.suffix.lower() == ".gpr":
            continue
        data = path.read_bytes()
        item = dict(path=path.relative_to(original).as_posix(), size=len(data),
                    sha256=hashlib.sha256(data).hexdigest())
        if path.suffix.lower() in (".exe", ".dll"):
            item["pe"] = PE(path).metadata()
            item["source_traces"] = sorted({s.decode("ascii", "replace") for s in
                re.findall(rb"[A-Za-z0-9_\\./-]+\.cpp", data)})
            if b"Nullsoft Install System v2.28" in data:
                item["installer_marker"] = "Nullsoft Install System v2.28"
        if path.suffix.lower() == ".pack":
            with zipfile.ZipFile(path) as archive:
                entries = []
                for info in archive.infolist():
                    entries.append(dict(name=info.filename, size=info.file_size,
                                        compressed_size=info.compress_size,
                                        crc32=f"{info.CRC:08x}", method=info.compress_type,
                                        encrypted=bool(info.flag_bits & 1)))
                item["archive"] = dict(format="ZIP", entries=entries,
                    file_types=dict(Counter(Path(e["name"]).suffix.lower() for e in entries
                                            if not e["name"].endswith("/"))))
        files.append(item)
    return dict(schema_version=1, files=files)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--original-dir", type=Path,
                        default=ROOT.parent / "farmář" / "Dobrý farmář")
    parser.add_argument("--output", type=Path, default=ROOT / "analysis" / "inventory.json")
    args = parser.parse_args()
    original = args.original_dir.resolve(strict=True)
    result = inspect(original)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Inventoried {len(result['files'])} files -> {args.output}")


if __name__ == "__main__":
    main()
