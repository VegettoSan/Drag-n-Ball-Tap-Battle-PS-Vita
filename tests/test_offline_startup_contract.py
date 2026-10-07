from pathlib import Path
import unittest

ROOT = Path(__file__).parents[1]


class OfflineStartupContractTest(unittest.TestCase):
    def test_downloader_finishes_obsolete_requests_immediately(self):
        source = (ROOT / "tools/aot/engine/java/com/namcobandaigames/dragonballtap/apk/Downloader.java").read_text()
        # Original Android semantics: true means the DownloadTask is still busy.
        # Vita is offline, so an obsolete request must complete immediately.
        self.assertIn("public boolean isDownload(){return false;}", source)
        self.assertIn("public byte[] GetData(){return null;}", source)
        self.assertIn("public int GetSize(){return 0;}", source)

    def test_vita_startup_uses_presence_scan_not_deep_pac_audit(self):
        resources = (ROOT / "tools/aot/engine/native/resources.cpp").read_text()
        self.assertIn("installed_audit = scanInstalledData(*vfs);", resources)
        self.assertNotIn("installed_audit = auditInstalledData(*vfs);", resources)


if __name__ == "__main__":
    unittest.main()
