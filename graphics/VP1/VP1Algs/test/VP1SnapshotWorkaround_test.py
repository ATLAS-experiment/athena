# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Test early policy, failure fallback and real process restart without Athena/GL."""
import contextlib
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

from VP1Algs import VP1SnapshotWorkaround as module


class SnapshotWorkaroundTest(unittest.TestCase):
    def setUp(self):
        self.environment = patch.dict(os.environ, {"PATH": os.defpath}, clear=True)
        self.environment.start()
        self.addCleanup(self.environment.stop)
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.prefix = Path(temporary.name)
        self.launcher = self.prefix / "bin/vp1"
        self.launcher.parent.mkdir()
        self.launcher.touch()
        self.support = self.prefix / "share/VP1Algs/offscreen-workaround"
        self.support.mkdir(parents=True)
        self.script = self.support / "vp1-with-glx-workaround"
        self.script.touch()
        self.library = self.prefix / "shim.so"
        self.library.touch()
        for name, value in (("system", "Linux"), ("machine", "aarch64"),
                            ("libc_ver", ("glibc", "2.34"))):
            mock = patch.object(module.platform, name, return_value=value)
            mock.start()
            self.addCleanup(mock.stop)

    def invoke(self, *arguments):
        with patch.object(sys, "argv", [str(self.launcher), *arguments]), \
                contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            module.enableSnapshotWorkaround()

    def prepared(self):
        return subprocess.CompletedProcess([], 0, str(self.library) + "\n", "")

    def test_local_help_and_end_of_options_never_prepare(self):
        for args in ([], ["--no-remote"], ["--", "--remote", "--fix-snapshots"],
                     ["--help", "--remote", "--fix-snapshots"], ["-h", "--fix-snapshots"]):
            with self.subTest(args=args), patch.object(module.subprocess, "run") as prepare, \
                    patch.object(os, "execve") as execute:
                self.invoke(*args)
                prepare.assert_not_called()
                execute.assert_not_called()

    def test_remote_auto_activation_preserves_arguments_and_preloads(self):
        os.environ["SSH_CONNECTION"] = "client server"
        os.environ["LD_PRELOAD"] = "/existing/library.so"
        args = ["file with spaces.root", "--phaseII", "--", "--no-fix-snapshots"]
        with patch.object(module.subprocess, "run", return_value=self.prepared()) as prepare, \
                patch.object(os, "execve", side_effect=RuntimeError("restarted")) as execute:
            with self.assertRaisesRegex(RuntimeError, "restarted"):
                self.invoke(*args)
        self.assertEqual(prepare.call_args.args[0], ["/bin/bash", str(self.script), "--prepare-only"])
        executable, arguments, environment = execute.call_args.args
        self.assertEqual(executable, sys.executable)
        self.assertEqual(arguments, [sys.executable, str(self.launcher), *args])
        self.assertEqual(environment["LD_PRELOAD"], str(self.library) + ":/existing/library.so")
        self.assertEqual(environment["VP1_GLX_WORKAROUND_LIBRARY"], str(self.library))
        self.assertEqual(environment["VP1_REMOTE_MODE"], "1")
        self.assertEqual(environment["COIN_FULL_INDIRECT_RENDERING"], "1")

    def test_explicit_fix_can_be_used_in_normal_mode(self):
        with patch.object(module.subprocess, "run", return_value=self.prepared()), \
                patch.object(os, "execve", side_effect=RuntimeError("restarted")) as execute:
            with self.assertRaisesRegex(RuntimeError, "restarted"):
                self.invoke("--fix-snapshots", "--no-remote")
        self.assertEqual(execute.call_args.args[2]["VP1_REMOTE_MODE"], "0")

    def test_opt_out_retains_remote_mode(self):
        with patch.object(module.subprocess, "run") as prepare:
            self.invoke("--remote", "--no-fix-snapshots")
        prepare.assert_not_called()
        self.assertEqual(os.environ["VP1_REMOTE_MODE"], "1")
        self.assertEqual(os.environ["VP1_SNAPSHOT_WORKAROUND_STATUS"], "disabled")

    def test_conflicting_flags_fail_before_preparing(self):
        for args in (("--remote", "--no-remote"), ("--fix-snapshots", "--no-fix-snapshots")):
            with patch.object(module.subprocess, "run") as prepare, self.assertRaises(SystemExit) as error:
                self.invoke(*args)
            self.assertEqual(error.exception.code, 2)
            prepare.assert_not_called()

    def test_loaded_library_is_restart_guard_even_with_opt_out(self):
        os.environ["VP1_GLX_WORKAROUND_LIBRARY"] = str(self.library)
        os.environ["LD_PRELOAD"] = str(self.library)
        with patch.object(module.subprocess, "run") as prepare, patch.object(os, "execve") as execute, \
                patch.object(module.ctypes, "CDLL"):
            self.invoke("--remote", "--no-fix-snapshots")
        prepare.assert_not_called()
        execute.assert_not_called()
        self.assertEqual(os.environ["VP1_SNAPSHOT_WORKAROUND_STATUS"], "active")
        self.assertIn("preloaded externally", os.environ["VP1_SNAPSHOT_WORKAROUND_MESSAGE"])

    def test_failed_preload_is_reported_without_restart_loop(self):
        os.environ["VP1_GLX_WORKAROUND_LIBRARY"] = str(self.library)
        os.environ["LD_PRELOAD"] = str(self.library)
        os.environ["VP1_GLX_WORKAROUND_ATTEMPT_PID"] = str(os.getpid())
        with patch.object(module.ctypes, "CDLL", side_effect=OSError("load failed")), \
                patch.object(module.subprocess, "run") as prepare, patch.object(os, "execve") as execute:
            self.invoke("--remote")
        prepare.assert_not_called()
        execute.assert_not_called()
        self.assertEqual(os.environ["VP1_SNAPSHOT_WORKAROUND_STATUS"], "unavailable")
        self.assertIn("dynamic loader", os.environ["VP1_SNAPSHOT_WORKAROUND_MESSAGE"])

    def test_stale_status_is_not_restart_guard(self):
        os.environ["VP1_GLX_WORKAROUND_LIBRARY"] = str(self.library)
        os.environ["VP1_SNAPSHOT_WORKAROUND_STATUS"] = "active"
        with patch.object(module.subprocess, "run", return_value=self.prepared()), \
                patch.object(os, "execve", side_effect=RuntimeError("restarted")):
            with self.assertRaisesRegex(RuntimeError, "restarted"):
                self.invoke("--remote")

    def test_unsupported_automatic_mode_continues_but_explicit_fix_fails(self):
        with patch.object(module.platform, "system", return_value="Darwin"), \
                patch.object(module.subprocess, "run") as prepare:
            self.invoke("--remote")
            self.assertEqual(os.environ["VP1_SNAPSHOT_WORKAROUND_STATUS"], "unavailable")
            with self.assertRaises(SystemExit):
                self.invoke("--remote", "--fix-snapshots")
        prepare.assert_not_called()

    def test_missing_support_is_nonfatal_in_automatic_mode(self):
        self.script.unlink()
        self.invoke("--remote")
        self.assertIn("support files", os.environ["VP1_SNAPSHOT_WORKAROUND_MESSAGE"])
        self.assertEqual(os.environ["VP1_REMOTE_MODE"], "1")

    def test_compilation_failure_is_nonfatal_and_reported(self):
        failed = subprocess.CompletedProcess([], 1, "", "compiler detail\nmissing development headers\n")
        with patch.object(module.subprocess, "run", return_value=failed), \
                patch.object(os, "execve") as execute:
            self.invoke("--remote")
        execute.assert_not_called()
        self.assertEqual(os.environ["VP1_SNAPSHOT_WORKAROUND_STATUS"], "unavailable")
        self.assertIn("missing development headers", os.environ["VP1_SNAPSHOT_WORKAROUND_MESSAGE"])

    def test_timeout_or_exec_failure_is_nonfatal(self):
        with patch.object(module.subprocess, "run", side_effect=subprocess.TimeoutExpired("compiler", 30)):
            self.invoke("--remote")
        self.assertEqual(os.environ["VP1_SNAPSHOT_WORKAROUND_STATUS"], "unavailable")
        with patch.object(module.subprocess, "run", return_value=self.prepared()), \
                patch.object(os, "execve", side_effect=OSError("cannot execute")):
            self.invoke("--remote")
        self.assertIn("cannot execute", os.environ["VP1_SNAPSHOT_WORKAROUND_MESSAGE"])
        self.assertNotIn("LD_PRELOAD", os.environ)

    @unittest.skipUnless(sys.platform.startswith("linux") and shutil.which("cc"), "Linux C compiler required")
    def test_real_exec_restarts_once_and_preserves_flags(self):
        # A harmless real shared library lets the dynamic loader run without GL/X11.
        source = self.prefix / "empty.c"
        source.write_text("int vp1_test_library;\n")
        subprocess.run(["cc", "-shared", "-fPIC", str(source), "-o", str(self.library)], check=True)
        counter = self.prefix / "preparations"
        self.script.write_text(
            f'#!/bin/bash\nprintf x >> "{counter}"\nprintf "%s\\n" "{self.library}"\n')
        self.launcher.write_text(
            "from VP1Algs.VP1SnapshotWorkaround import enableSnapshotWorkaround\n"
            "import json, os, sys\nenableSnapshotWorkaround()\n"
            "print(json.dumps(dict(status=os.environ['VP1_SNAPSHOT_WORKAROUND_STATUS'], "
            "mode=os.environ['VP1_REMOTE_MODE'], coin=os.environ['COIN_FULL_INDIRECT_RENDERING'], "
            "args=sys.argv[1:])))\n")
        # Restore only the installed module search path needed by the subprocess.
        environment = os.environ.copy()
        environment["PYTHONPATH"] = os.pathsep.join(sys.path)
        result = subprocess.run([sys.executable, str(self.launcher), "--remote", "file with spaces.root"],
                                env=environment, capture_output=True, text=True, timeout=15, check=True)
        data = json.loads(result.stdout.splitlines()[-1])
        self.assertEqual(data, dict(status="active", mode="1", coin="1", args=["--remote", "file with spaces.root"]))
        self.assertEqual(counter.read_text(), "x")
        environment["COIN_FULL_INDIRECT_RENDERING"] = "0"
        opted_out = subprocess.run([sys.executable, str(self.launcher), "--remote"], env=environment,
                                   capture_output=True, text=True, timeout=15, check=True)
        self.assertEqual(json.loads(opted_out.stdout.splitlines()[-1])["coin"], "0")
        self.assertEqual(counter.read_text(), "xx")
        # The loader ignores an invalid preload. It must be reported, without another restart.
        self.library.write_text("invalid shared library")
        failed = subprocess.run([sys.executable, str(self.launcher), "--remote"], env=environment,
                                capture_output=True, text=True, timeout=15, check=True)
        data = json.loads(failed.stdout.splitlines()[-1])
        self.assertEqual(data["status"], "unavailable")
        self.assertEqual(counter.read_text(), "xxx")

    @unittest.skipUnless(sys.platform.startswith("linux") and shutil.which("cc"), "Linux C compiler required")
    def test_real_wrapper_prepare_only_and_cached_library(self):
        script = Path(module.__file__).resolve().parents[1] / "share/offscreen-workaround/vp1-with-glx-workaround"
        os.environ["VP1_GLX_WORKAROUND_CACHE_DIR"] = str(self.prefix / "cache")
        command = ["/bin/bash", str(script), "--prepare-only"]
        first = subprocess.run(command, capture_output=True, text=True, timeout=30, check=True)
        library = Path(first.stdout.strip())
        self.assertTrue(library.is_file())
        # Cached libraries do not require a compiler on subsequent launches.
        os.environ["CC"] = "vp1-nonexistent-compiler"
        second = subprocess.run(command, capture_output=True, text=True, timeout=15, check=True)
        self.assertEqual(second.stdout, first.stdout)

        # Exercise the actual shim plus the Python restart guard without creating a GL context.
        shutil.copyfile(script, self.script)
        shutil.copyfile(script.parent / "vp1-glx-pixel-limit.c", self.support / "vp1-glx-pixel-limit.c")
        self.launcher.write_text(
            "from VP1Algs.VP1SnapshotWorkaround import enableSnapshotWorkaround\n"
            "import os\nenableSnapshotWorkaround()\n"
            "print('STATUS=' + os.environ['VP1_SNAPSHOT_WORKAROUND_STATUS'])\n")
        environment = os.environ.copy()
        environment["PYTHONPATH"] = os.pathsep.join(sys.path)
        launched = subprocess.run([sys.executable, str(self.launcher), "--remote"], env=environment,
                                  capture_output=True, text=True, timeout=15, check=True)
        self.assertIn("STATUS=active", launched.stdout)
        library.unlink()
        failed = subprocess.run(command, capture_output=True, text=True, timeout=15)
        self.assertNotEqual(failed.returncode, 0)
        self.assertIn("C compiler is required", failed.stderr)


if __name__ == "__main__":
    unittest.main()
