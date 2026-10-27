# Copyright (C) 2026 CERN for the benefit of the ATLAS collaboration
"""Test early launcher re-execution without loading Athena or the GUI."""
import importlib.util
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

source = Path(__file__).resolve().parents[1] / "python/VP1SnapshotWorkaround.py"
spec = importlib.util.spec_from_file_location("workaround", source)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class SnapshotWorkaroundTest(unittest.TestCase):
    def test_opt_in_and_help(self):
        for args in ([], ["--help", "--fix-snapshots"],
                     ["--", "--fix-snapshots"]):
            with patch.object(sys, "argv", ["vp1", *args]), patch.object(os, "execv") as execute:
                module.enableSnapshotWorkaround()
                execute.assert_not_called()

    def test_installed_restart_preserves_arguments(self):
        with tempfile.TemporaryDirectory() as temporary:
            prefix = Path(temporary)
            launcher = prefix / "bin/vp1"
            support = prefix / "share/VP1Algs/offscreen-workaround"
            support.mkdir(parents=True)
            (support / "vp1-with-glx-workaround").touch()
            arguments = [str(launcher), "--fix-snapshots", "file with spaces.root", "--phaseII"]
            with patch.object(sys, "argv", arguments), \
                    patch.object(os, "execv", side_effect=RuntimeError("restarted")) as execute:
                with self.assertRaisesRegex(RuntimeError, "restarted"):
                    module.enableSnapshotWorkaround()
                execute.assert_called_once_with("/bin/bash", [
                    "bash", str(support / "vp1-with-glx-workaround"),
                    "--vp1-exec", sys.executable, str(launcher),
                    "file with spaces.root", "--phaseII"])


if __name__ == "__main__":
    unittest.main()
