#!/usr/bin/env python3
"""Compare recovered String search/compare with pure routines in the original EXE."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--oracle", type=Path)
    parser.add_argument("--wide", action="store_true", help="Compare pure UTF-16 WString ordering")
    parser.add_argument("--original", type=Path, default=ROOT.parent / "farmář/Dobrý farmář/Dobrý farmář.exe")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    kind = "WString" if args.wide else "String"
    args.oracle = args.oracle or ROOT / f"build/windows/Release/{'w' if args.wide else ''}string-reference-test.exe"
    args.output = args.output or ROOT / f".local/{'w' if args.wide else ''}string-validation.json"
    inventory = json.loads((ROOT / "analysis/inventory.json").read_text(encoding="utf-8"))
    expected = next(row["sha256"] for row in inventory["files"] if row["path"] == "Dobrý farmář.exe")
    original = args.original.resolve(strict=True)
    if hashlib.sha256(original.read_bytes()).hexdigest() != expected:
        parser.error("Original EXE does not match the inventoried edition; addresses cannot be used")
    result = subprocess.run([str(args.oracle.resolve(strict=True)), str(original)],
                            capture_output=True, text=True, timeout=60)
    report = dict(recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  original_sha256=expected, oracle_sha256=hashlib.sha256(args.oracle.read_bytes()).hexdigest(),
                  routines={"compare": "00401050"} if args.wide else {"find": "00548260", "rfind": "00548330", "compare": "00403960"},
                  seed="57464152" if args.wide else "4641524d", trials=20000,
                  comparisons=int(result.stdout.strip()) if result.returncode == 0 else 0,
                  status="matches_original" if result.returncode == 0 else "failed",
                  exit_code=result.returncode, error=result.stderr.strip())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"{kind}: {report['status']}, {report['comparisons']} comparisons")
    if result.returncode:
        print(result.stderr.strip())
        raise SystemExit(1)


if __name__ == "__main__":
    main()
