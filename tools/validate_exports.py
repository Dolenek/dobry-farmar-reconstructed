#!/usr/bin/env python3
"""Check a rebuilt DLL's architecture, decorated export names and ordinals."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
from pe import PE

ROOT = Path(__file__).resolve().parents[1]


def export_contract(exports):
    return sorted((row["ordinal"], tuple(sorted(row["names"])), row.get("forwarder")) for row in exports)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--original-name", choices=["JNGLoad.dll", "Squall.dll"], required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    inventory = json.loads((ROOT / "analysis/inventory.json").read_text(encoding="utf-8"))
    original = next(row for row in inventory["files"] if row["path"] == args.original_name)
    binary = args.binary.resolve(strict=True)
    actual = PE(binary).metadata()
    digest = hashlib.sha256(binary.read_bytes()).hexdigest()
    failures = []
    if digest == original["sha256"]:
        failures.append("Binary is the unchanged original, not a reconstruction")
    for field in ("bits", "machine"):
        if actual[field] != original["pe"][field]:
            failures.append(f"Architecture {field} differs from original")
    if export_contract(actual["exports"]) != export_contract(original["pe"]["exports"]):
        failures.append("Export names, ordinals or forwarders differ from original")
    if not PE(binary).characteristics & 0x2000:
        failures.append("PE image is not marked as a DLL")
    report = dict(recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  status="matches_export_contract" if not failures else "failed",
                  original_name=args.original_name, original_sha256=original["sha256"],
                  rebuilt_sha256=digest, bits=actual["bits"], machine=actual["machine"],
                  exports=actual["exports"], failures=failures,
                  note="Checks the PE export contract, not runtime behavior or exception ABI")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"{args.original_name}: {report['status']}")
    if failures:
        print("\n".join(failures))
        raise SystemExit(1)


if __name__ == "__main__":
    main()
