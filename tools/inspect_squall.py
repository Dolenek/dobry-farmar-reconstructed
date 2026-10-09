#!/usr/bin/env python3
"""Compare a hash-pinned Squall reference source with the original export inventory."""
import argparse
import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-dir", type=Path, default=ROOT / ".local" / "squall-source")
    args = parser.parse_args()
    lock = json.loads((ROOT / "analysis" / "squall-source.lock.json").read_text())
    for item in lock["files"]:
        source = args.source_dir / item["path"]
        if hashlib.sha256(source.read_bytes()).hexdigest() != item["sha256"]:
            raise ValueError(f"Reference source hash mismatch: {source}")
    header = (args.source_dir / "source" / "Squall.h").read_text(encoding="utf-8")
    code = (args.source_dir / "source" / "SoundEngine" / "Squall.cpp").read_text(encoding="utf-8")
    declarations = {}
    for match in re.finditer(r"DLL_API\s+\w+\s+(SQUALL_\w+)\s*\([^;]*\);", header):
        declarations[match.group(1)] = dict(reference_signature=" ".join(match.group(0).split()),
                                           header_line=header[:match.start()].count("\n") + 1)
    inventory = json.loads((ROOT / "analysis" / "inventory.json").read_text(encoding="utf-8"))
    exports = next(f for f in inventory["files"] if f["path"] == "Squall.dll")["pe"]["exports"]
    rows = []
    for export in exports:
        name = export["names"][0]
        offset = code.find(" " + name + "(")
        rows.append(dict(name=name, address=export["address"], ordinal=export["ordinal"],
                         **declarations.get(name, {}),
                         implementation_line=code[:offset].count("\n") + 1 if offset >= 0 else None))
    original = {e["names"][0] for e in exports}
    report = dict(status="reference_source_differs_from_original", source_commit=lock["commit"],
                  original_export_count=len(exports), reference_declaration_count=len(declarations),
                  missing_declarations=sorted(original - set(declarations)),
                  extra_declarations=sorted(set(declarations) - original),
                  verified_implementation_differences=[
                      "Original volume table uses trunc(10000 * log10(0.009F * i + 0.1F)); "
                      "reference uses floor(5000 * log10(i / 99.0F))."],
                  original_runtime_compared=False, exports=rows)
    output = ROOT / "analysis" / "squall-source-match.json"
    output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"Mapped {len(rows)} original exports; source variant differs from original")


if __name__ == "__main__":
    main()
