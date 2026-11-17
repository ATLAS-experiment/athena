# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Exercise launch decisions without loading Athena or opening a display."""
import contextlib
import importlib.util
import io
import os
from pathlib import Path
import unittest
from unittest.mock import patch

source = Path(__file__).resolve().parents[1] / "python/VP1RemoteMode.py"
spec = importlib.util.spec_from_file_location("remote", source)
remote = importlib.util.module_from_spec(spec)
spec.loader.exec_module(remote)


class RemoteModeTest(unittest.TestCase):
    def test_local_displays(self):
        for display in ("", ":0", "unix:0", "localhost:0", "workstation:0",
                        "workstation/unix:0", "127.0.0.1:9", "[::1]:0",
                        "/private/tmp/com.apple.launchd/X:0"):
            with self.subTest(display=display):
                self.assertFalse(remote.remoteSessionReasons({"DISPLAY": display}, "workstation"))

    def test_remote_indicators(self):
        for environment in ({"SSH_CONNECTION": "client server"},
                            {"SSH_CLIENT": "client"},
                            {"DISPLAY": "localhost:10.0"},
                            {"DISPLAY": "tcp/otherhost:0"},
                            {"DISPLAY": "[::1]:12"},
                            {"XRDP_SESSION": "1"}, {"NXSESSIONID": "session"},
                            {"X2GO_SESSION": "session"}, {"VNCSESSION": "session"}):
            with self.subTest(environment=environment):
                self.assertTrue(remote.remoteSessionReasons(environment, "workstation"))

    def test_reasons_are_specific_and_not_duplicated(self):
        reasons = remote.remoteSessionReasons(
            {"SSH_CLIENT": "client", "SSH_CONNECTION": "client server",
             "DISPLAY": "localhost:10.0", "VNCDESKTOP": "desktop"}, "workstation")
        self.assertEqual(reasons, ["SSH session", "remote desktop session", "forwarded X11 display"])

    def test_local_hostname_and_empty_session_markers(self):
        for display in ("WORKSTATION:0", "workstation.example:0", "workstation:0"):
            self.assertFalse(remote.remoteSessionReasons(
                {"DISPLAY": display, "SSH_CLIENT": "", "XRDP_SESSION": ""}, "workstation.example"))

    def test_override_clears_inherited_mode(self):
        with patch.dict(os.environ, {"SSH_CONNECTION": "client server",
                                    "VP1_REMOTE_MODE": "1", "VP1_REMOTE_REASON": "old"}, clear=True):
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertFalse(remote.configureRemoteMode(no_remote=True))
            self.assertEqual(os.environ["VP1_REMOTE_MODE"], "0")
            self.assertEqual(os.environ["VP1_REMOTE_REASON"], "")
            self.assertEqual(output.getvalue(), "")

    def test_force_remote_skips_detection(self):
        with patch.dict(os.environ, {}, clear=True), \
                patch.object(remote, "remoteSessionReasons") as detect, \
                patch.object(remote.socket, "gethostname") as hostname:
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertTrue(remote.configureRemoteMode(force_remote=True))
            detect.assert_not_called()
            hostname.assert_not_called()
            self.assertEqual(os.environ["VP1_REMOTE_MODE"], "1")
            self.assertEqual(os.environ["VP1_REMOTE_REASON"], "requested with --remote")
            self.assertIn("requested with --remote", output.getvalue())
            self.assertIn("--no-remote", output.getvalue())

    def test_conflicting_overrides_do_not_change_environment(self):
        with patch.dict(os.environ, {}, clear=True):
            with self.assertRaisesRegex(ValueError, "cannot be used together"):
                remote.configureRemoteMode(no_remote=True, force_remote=True)
            self.assertNotIn("VP1_REMOTE_MODE", os.environ)

    def test_remote_defaults_to_full_indirect_features(self):
        with patch.dict(os.environ, {}, clear=True):
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                remote.configureRemoteMode(force_remote=True)
            self.assertEqual(os.environ["COIN_FULL_INDIRECT_RENDERING"], "1")
            self.assertIn("OpenGL 1.1+ features allowed", output.getvalue())
            self.assertIn("Coin OpenGL policy:", output.getvalue())

    def test_normal_mode_does_not_set_or_clear_coin_values(self):
        with patch.dict(os.environ, {}, clear=True):
            remote.configureRemoteMode(no_remote=True)
            self.assertNotIn("COIN_FULL_INDIRECT_RENDERING", os.environ)
        with patch.dict(os.environ, {"COIN_FULL_INDIRECT_RENDERING": "1"}, clear=True):
            remote.configureRemoteMode(no_remote=True)
            self.assertEqual(os.environ["COIN_FULL_INDIRECT_RENDERING"], "1")

    def test_remote_preserves_explicit_coin_values(self):
        for value in ("0", "1", "2", "", "-1", "invalid"):
            with self.subTest(value=value), \
                    patch.dict(os.environ, {"COIN_FULL_INDIRECT_RENDERING": value}, clear=True):
                remote.configureRemoteMode(force_remote=True, announce=False)
                self.assertEqual(os.environ["COIN_FULL_INDIRECT_RENDERING"], value)
                message = os.environ["VP1_COIN_INDIRECT_RENDERING_MESSAGE"]
                self.assertIn(repr(value), message)
                self.assertEqual("features allowed" in message, value in ("1", "2"))

    def test_force_gl1_takes_precedence(self):
        for full in (None, "1", "0"):
            values = {"COIN_FORCE_GL1_0_ONLY": "1"}
            if full is not None:
                values["COIN_FULL_INDIRECT_RENDERING"] = full
            with self.subTest(full=full), patch.dict(os.environ, values, clear=True):
                remote.configureRemoteMode(force_remote=True, announce=False)
                self.assertEqual(os.environ.get("COIN_FULL_INDIRECT_RENDERING"), full)
                self.assertEqual(os.environ["COIN_FORCE_GL1_0_ONLY"], "1")
                self.assertIn("OpenGL 1.0 only", os.environ["VP1_COIN_INDIRECT_RENDERING_MESSAGE"])
        with patch.dict(os.environ, {"COIN_FORCE_GL1_0_ONLY": "0"}, clear=True):
            remote.configureRemoteMode(force_remote=True, announce=False)
            self.assertEqual(os.environ["COIN_FULL_INDIRECT_RENDERING"], "1")

    def test_bootstrap_decision_survives_display_environment_changes(self):
        with patch.dict(os.environ, {"DISPLAY": "localhost:10.0"}, clear=True):
            remote.configureRemoteMode(announce=False)
            os.environ["VP1_REMOTE_BOOTSTRAP"] = remote.remoteBootstrapKey()
            os.environ["DISPLAY"] = ":0"
            with patch.object(remote, "remoteSessionReasons") as detect:
                self.assertTrue(remote.configureRemoteMode(announce=False, reuse_bootstrap=True))
                detect.assert_not_called()
            # A new process must not inherit a previous process's detection decision.
            with patch.object(os, "getpid", return_value=os.getpid() + 1):
                self.assertFalse(remote.configureRemoteMode(announce=False, reuse_bootstrap=True))

    def test_remote_notice_and_local_reset(self):
        with patch.dict(os.environ, {"DISPLAY": "localhost:10.0"}, clear=True):
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertTrue(remote.configureRemoteMode())
            self.assertEqual(os.environ["VP1_REMOTE_MODE"], "1")
            self.assertIn("forwarded X11", output.getvalue())
            self.assertIn("--no-remote", output.getvalue())
            os.environ["DISPLAY"] = ":0"
            self.assertFalse(remote.configureRemoteMode())
            self.assertEqual(os.environ["VP1_REMOTE_MODE"], "0")


if __name__ == "__main__":
    unittest.main()
