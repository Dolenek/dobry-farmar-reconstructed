"""Verify recovered removal control flow and Czech string escaping from the original."""
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from nsis_extract import extract

INPUT = ROOT.parent / "farmář" / "Dobrý farmář" / "Uninstall.exe"


@unittest.skipUnless(INPUT.is_file(), "Original uninstaller is local-only")
class UninstallerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.program = extract(INPUT)

    def test_removal_section_and_no_execution(self):
        self.assertEqual(len(self.program["instructions"]), 137)
        self.assertEqual(self.program["sections"][0]["first_instruction"], 47)
        self.assertEqual(self.program["sections"][0]["instruction_count"], 27)
        first = self.program["instructions"][46]
        self.assertEqual(first["operation"], "RMDIR")
        self.assertEqual(first["strings"]["0"], "$INSTDIR")
        self.assertEqual(first["arguments"][1], 3)  # DEL_DIR | DEL_RECURSE

    def test_czech_name_and_registry_semantics(self):
        strings = {s["offset"]: s["text"] for s in self.program["strings"]}
        self.assertEqual(strings[217], "Dobrý farmář")
        self.assertEqual(strings[245], "$DESKTOP\\Dobrý farmář.lnk")
        self.assertEqual(strings[231], "$SMPROGRAMS\\$var34\\*.lnk")
        delete_game, delete_uninstall = self.program["instructions"][71:73]
        self.assertEqual(delete_game["operation"], "DELREG")
        self.assertEqual(delete_game["arguments"][4], 3)  # DeleteRegKey /ifempty
        self.assertEqual(delete_uninstall["arguments"][4], 1)  # unconditional DeleteRegKey
        self.assertEqual(delete_uninstall["strings"]["2"],
                         "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Dobrý farmář")


if __name__ == "__main__":
    unittest.main()
