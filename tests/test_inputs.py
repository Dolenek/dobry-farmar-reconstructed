"""Independent checks against objdump and ZIP CRCs for the supplied reference."""
import hashlib
import json
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from pe import PE, PEError

ORIGINAL = ROOT.parent / "farmář" / "Dobrý farmář"


class ReferenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not ORIGINAL.is_dir():
            raise unittest.SkipTest("Original CD files are local-only and not present")
        cls.inventory = json.loads((ROOT / "analysis" / "inventory.json").read_text(encoding="utf-8"))

    def test_original_files_still_match_inventory(self):
        for item in self.inventory["files"]:
            with self.subTest(file=item["path"]):
                data = (ORIGINAL / item["path"]).read_bytes()
                self.assertEqual(len(data), item["size"])
                self.assertEqual(hashlib.sha256(data).hexdigest(), item["sha256"])

    @unittest.skipUnless(shutil.which("objdump"), "objdump is required for independent comparison")
    def test_pe_imports_and_exports_match_objdump(self):
        for item in self.inventory["files"]:
            if "pe" not in item:
                continue
            with self.subTest(file=item["path"]):
                path = ORIGINAL / item["path"]
                # Windows binutils may use ANSI argv and cannot open Czech paths.
                # Compare an unchanged copy using an ASCII basename and Unicode cwd.
                with tempfile.TemporaryDirectory(prefix="farmar-pe-") as folder:
                    reference = Path(folder) / "reference.exe"
                    shutil.copyfile(path, reference)
                    result = subprocess.run(["objdump", "-p", reference.name], cwd=folder,
                                            check=True, capture_output=True, text=True)
                pe = PE(path)
                libraries = re.findall(r"DLL Name: (\S+)", result.stdout)
                self.assertEqual(libraries, [x["library"] for x in pe.imports()])
                table = re.search(r"\[Ordinal/Name Pointer\] Table[^\n]*\n(.*?)(?:\n\n|$)",
                                  result.stdout, re.S)
                names = re.findall(r"\[\s*\d+\]\s+(?:\+base\[\s*\d+\]\s+[0-9a-fA-F]+\s+)?(\S+)",
                                   table.group(1)) if table else []
                self.assertEqual(sorted(names), sorted(n for e in pe.exports() for n in e["names"]))

    def test_all_archive_crcs(self):
        for name in ("Data/Data.pack", "Data/cz.pack"):
            with self.subTest(archive=name), zipfile.ZipFile(ORIGINAL / name) as archive:
                self.assertIsNone(archive.testzip(), "Corrupt archive entry")

    def test_truncated_pe_headers_fail_cleanly(self):
        data = (ORIGINAL / "JNGLoad.dll").read_bytes()
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "truncated.dll"
            for size in (0, 2, 60, 64, 128, 256):
                with self.subTest(size=size):
                    path.write_bytes(data[:size])
                    with self.assertRaises(PEError):
                        PE(path)


if __name__ == "__main__":
    unittest.main()
