import json
import unittest
from unittest import mock

from avmdbg.cli import build_parser, main
from avmdbg.jdwp.errors import AdbError


class ParserTest(unittest.TestCase):
    def test_break_requires_core_options(self):
        parser = build_parser()
        with self.assertRaises(SystemExit):
            parser.parse_args(["break", "com.example.app"])

    def test_break_index_accepts_hex(self):
        args = build_parser().parse_args(
            [
                "break",
                "com.example.app",
                "--class",
                "Lcom/example/A;",
                "--method",
                "m",
                "--sign",
                "()V",
                "--registers",
                "2",
                "--index",
                "0x10",
            ]
        )
        self.assertEqual(args.index, 16)

    def test_reg_spec(self):
        args = build_parser().parse_args(
            [
                "break",
                "com.example.app",
                "--class",
                "Lcom/example/A;",
                "--method",
                "m",
                "--sign",
                "()V",
                "--registers",
                "2",
                "--reg",
                "v0:I",
            ]
        )
        self.assertEqual(args.reg, [("v0", "I")])

    def test_bad_reg_spec_rejected(self):
        with self.assertRaises(SystemExit):
            build_parser().parse_args(
                [
                    "break",
                    "com.example.app",
                    "--class",
                    "Lcom/example/A;",
                    "--method",
                    "m",
                    "--sign",
                    "()V",
                    "--registers",
                    "2",
                    "--reg",
                    "v0",
                ]
            )


class CliErrorPathsTest(unittest.TestCase):
    def test_adb_missing_is_json_error(self):
        with mock.patch(
            "avmdbg.cli.adb.list_devices", side_effect=AdbError("adb not found")
        ):
            import sys
            from io import StringIO

            stderr = StringIO()
            old = sys.stderr
            sys.stderr = stderr
            try:
                code = main(["devices"])
            finally:
                sys.stderr = old
            self.assertEqual(code, 2)
            payload = json.loads(stderr.getvalue())
            self.assertEqual(payload["error"]["type"], "adb")
            self.assertIn("adb not found", payload["error"]["message"])


if __name__ == "__main__":
    unittest.main()
