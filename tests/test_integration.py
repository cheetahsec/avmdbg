"""End-to-end tests against a real device.

Skipped unless the AVMDDBG_INTEGRATION environment variable is set to ``1``.
Requires: adb on PATH, one connected device, and the fixture app
(``tests/fixtures/app-debug.apk``) installed and running.
"""

import os
import unittest

from avmdbg import Debugger
from avmdbg import adb

PACKAGE = "com.example.x0r.demo"
FIXTURE_APK = os.path.join(os.path.dirname(__file__), "fixtures", "app-debug.apk")


@unittest.skipUnless(
    os.environ.get("AVMDDBG_INTEGRATION") == "1",
    "set AVMDDBG_INTEGRATION=1 with a connected device to run",
)
class IntegrationTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        adb.run_adb(["install", "-r", FIXTURE_APK])

    def test_attach_and_breakpoint(self):
        adb.set_debug_app(PACKAGE, wait=True)
        try:
            adb.launch_app(PACKAGE)
            import time

            time.sleep(3)
            with Debugger.attach(PACKAGE) as dbg:
                vm = dbg.version()
                self.assertIn("description", vm)
                bp = dbg.set_breakpoint(
                    class_signature="Lcom/example/x0r/demo/LoginActivity;",
                    method="test0",
                    signature="(B[IJLjava/lang/String;)V",
                    index=0,
                    registers=9,
                )
                self.assertGreater(bp.request_id, 0)
        finally:
            adb.clear_debug_app()


if __name__ == "__main__":
    unittest.main()
