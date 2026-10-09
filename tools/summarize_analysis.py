#!/usr/bin/env python3
"""Publish small coverage metadata, keeping full analysis databases and dumps local."""
import collections
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    programs = {}
    for name in ("game", "jngload", "squall", "uninstall"):
        raw = ROOT / "analysis" / "raw" / name
        summary = json.loads((raw / "summary.json").read_text(encoding="utf-8"))
        if summary["status"] != "export_complete":
            raise ValueError(f"Incomplete export: {name}")
        functions = [json.loads(line) for line in (raw / "functions.jsonl").read_text(encoding="utf-8").splitlines()]
        if len(functions) != summary["function_count"]:
            raise ValueError(f"Function index count differs: {name}")
        summary["thunk_count"] = sum(f["thunk"] for f in functions)
        summary["compiler_helper_counts"] = dict(collections.Counter(
            f["name"].split("@")[0] for f in functions if f["name"].startswith(("Catch@", "Unwind@"))))
        summary["decompilation_errors"] = [dict(address=f["address"], name=f["name"],
                                                 signature=f["signature"], error=f["error"])
                                            for f in functions if "error" in f]
        summary["reconstruction_complete"] = False
        summary["original_runtime_compared"] = False
        programs[name] = summary
    report = dict(schema_version=1, status="analysis_complete_reconstruction_incomplete", programs=programs)
    output = ROOT / "analysis" / "export-status.json"
    output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Summarized {len(programs)} program exports -> {output}")


if __name__ == "__main__":
    main()
