#!/usr/bin/env python3
"""Run Ghidra on an isolated working project, on Linux or Windows."""
from __future__ import annotations

import argparse
import getpass
import hashlib
import json
import os
import shutil
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BINARIES = {"game": "Dobrý farmář.exe", "uninstall": "Uninstall.exe",
            "squall": "Squall.dll", "jngload": "JNGLoad.dll"}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--ghidra-home", type=Path, required=True)
    p.add_argument("--java-home", type=Path)
    p.add_argument("--binary", choices=BINARIES, required=True)
    p.add_argument("--original-dir", type=Path, default=ROOT.parent / "farmář" / "Dobrý farmář")
    p.add_argument("--project-name", default="Dobrý Farmář")
    p.add_argument("--seed-project", type=Path, help="Existing .gpr; copied only when work project is absent")
    p.add_argument("--process", help="Existing program name or Ghidra wildcard, instead of import")
    p.add_argument("--no-analysis", action="store_true", help="Export an already analyzed program")
    p.add_argument("--repair-game-flow", action="store_true", help="Correct verified __chkstk/free no-return errors in the game copy")
    p.add_argument("--function-timeout", type=int, default=30)
    args = p.parse_args()
    if args.function_timeout < 1:
        p.error("--function-timeout must be positive")
    if args.no_analysis and not args.process:
        p.error("--no-analysis requires --process")
    if args.repair_game_flow and (args.binary != "game" or not args.process):
        p.error("--repair-game-flow requires --binary game and --process")
    # Ghidra rejects project paths containing dot-prefixed directory components.
    work = ROOT / "work" / "ghidra"
    work.mkdir(parents=True, exist_ok=True)
    project = work / (args.project_name + ".gpr")
    if args.seed_project and not project.exists():
        seed = args.seed_project.resolve(strict=True)
        if seed.stem != args.project_name:
            p.error("--project-name must match the seed project filename")
        shutil.copytree(seed.with_suffix(".rep"), work / (args.project_name + ".rep"))
        shutil.copy2(seed, project)
    if args.process and not project.exists():
        p.error("Working project does not exist; supply --seed-project or import first")
    properties = work / (args.project_name + ".rep") / "project.prp"
    if properties.exists():
        tree = ET.parse(properties)
        owner = tree.find(".//STATE[@NAME='OWNER']")
        if owner is not None and owner.get("VALUE") != getpass.getuser():
            # Only the local working copy is adopted; the seed project is untouched.
            owner.set("VALUE", getpass.getuser())
            tree.write(properties, encoding="utf-8", xml_declaration=True)
    output = ROOT / "analysis" / "raw" / args.binary
    output.mkdir(parents=True, exist_ok=True)
    headless = args.ghidra_home.resolve(strict=True) / "support" / (
        "analyzeHeadless.bat" if os.name == "nt" else "analyzeHeadless")
    if not headless.is_file():
        p.error(f"Headless launcher not found: {headless}")
    command = [str(headless), str(work), args.project_name]
    if args.process:
        command += ["-process", args.process]
    else:
        original = (args.original_dir / BINARIES[args.binary]).resolve(strict=True)
        command += ["-import", str(original)]
    if args.no_analysis:
        command += ["-noanalysis"]
    command += ["-max-cpu", "4", "-scriptPath", str(ROOT / "tools" / "ghidra")]
    if args.repair_game_flow:
        command += ["-preScript", "RepairGameFlow.java", str(output)]
    command += ["-postScript", "ExportReconstruction.java", str(output), str(args.function_timeout),
                "-log", str(output / "headless.log"), "-scriptlog", str(output / "script.log")]
    env = os.environ.copy()
    if args.java_home:
        env["JAVA_HOME"] = str(args.java_home.resolve(strict=True))
    # Keep Ghidra configuration and caches inside the ignored working directory.
    # Java option quotes are parsed by the JVM, not by a shell.
    settings = ROOT / ".local" / "ghidra-home"
    settings.mkdir(parents=True, exist_ok=True)
    extra = '"-Duser.home=' + str(settings) + '"'
    env["JAVA_TOOL_OPTIONS"] = (env.get("JAVA_TOOL_OPTIONS", "") + " " + extra).strip()
    env["XDG_CONFIG_HOME"] = str(settings / "config")
    env["XDG_CACHE_HOME"] = str(settings / "cache")
    print(f"Analyzing {args.binary}; exports -> {output}", flush=True)
    summary_path = output / "summary.json"
    summary_path.unlink(missing_ok=True)
    repair_path = output / "flow-repair.json"
    if args.repair_game_flow:
        repair_path.unlink(missing_ok=True)
    subprocess.run(command, env=env, check=True)
    # Ghidra may exit with code 0 even when the post-script fails to compile.
    if not summary_path.exists():
        raise RuntimeError("Ghidra did not produce an export summary; inspect headless.log")
    summary = json.loads(summary_path.read_text(encoding="utf-8"))
    if summary.get("status") != "export_complete":
        raise RuntimeError("Export did not complete; inspect summary.json and headless.log")
    if args.repair_game_flow and not repair_path.is_file():
        raise RuntimeError("Requested flow repair did not produce a report; inspect headless.log")
    original = args.original_dir / BINARIES[args.binary]
    if original.is_file():
        expected = hashlib.sha256(original.read_bytes()).hexdigest()
        if summary.get("input_sha256", "").lower() != expected:
            raise RuntimeError("Analyzed program does not match the selected original binary")
    print(f"Verified export: {summary['decompiled_count']}/{summary['function_count']} functions")


if __name__ == "__main__":
    main()
