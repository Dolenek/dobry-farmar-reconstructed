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
                      "reference uses floor(5000 * log10(i / 99.0F)).",
                      "Original channels are 300 bytes (100033d2..100033e5 and 100033fe..10003418). "
                      "Reference SSample has two additional end-worker fields at offsets 300 and 304, totaling 308 bytes.",
                      "Original InitParameters 100030a0 clamps DistanceFactor above 10 to 10. "
                      "Reference InitParameters lacks this upper limit.",
                      "Original InitParameters changes unordered (NaN) distance/rolloff/doppler values to 1. "
                      "Reference uses <=0 tests which retain NaNs.",
                      "Original volume table reserves 256 integers, zeroes the tail and calculates indices 0..99; reference reserves only 100.",
                      "Original Channel_GetVolume applies the inverse of its original curve (100056b0..100056d7); reference uses another inverse.",
                      "Original GetNumDevice tail-calls api_GetNumDevice at 10007e40; reference dereferences an uninitialized CDXOutput pointer.",
                      "Original decoder dispatch includes the WMA extension branch and tracker fallback (1002d8c0); these branches are disabled in reference source."],
                  instruction_evidence=dict(channel_allocation="100033d2..100033e5",channel_zeroing="100033fe..10003418",
                      parameter_normalization="100030a0..1000321d",factor_nan_tests=["10003170","10003198","100031c0"]),
                  reference_source_evidence=dict(channel_fields="source/SoundEngine/Channels.h:104",
                      end_callback="source/SoundEngine/Channels.cpp:746",end_callback_export="source/SoundEngine/Squall.cpp:4634",
                      parameter_validation="source/SoundEngine/Squall.cpp:133"),
                  original_runtime_compared=False, exports=rows)
    layout=ROOT/'analysis/squall-layout-validation.json'
    if layout.exists(): report['reference_layout_probe']=json.loads(layout.read_text(encoding='utf-8'))
    output = ROOT / "analysis" / "squall-source-match.json"
    output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"Mapped {len(rows)} original exports; source variant differs from original")


if __name__ == "__main__":
    main()
