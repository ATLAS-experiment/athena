# Copyright (C) 2026 CERN for the benefit of the ATLAS collaboration

"""Regression tests for VP1 configuration, without a GUI or geometry."""

import contextlib
import io
import os
import unittest
from unittest.mock import patch

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from VP1Algs.VP1AlgConfig import SetupVP1, VP1AlgCfg, VP1EventInfoCfg
from VP1Algs.VP1AlgsConf import VP1Alg
from GaudiKernel.GaudiHandles import PublicToolHandle


class VP1AlgConfigTest(unittest.TestCase):
    def setUp(self):
        self.flags = initConfigFlags()
        self.flags.lock()

    def algorithm(self, **kwargs):
        cfg = VP1AlgCfg(self.flags, **kwargs)
        self.addCleanup(cfg.wasMerged)
        return cfg.getPrimary()

    def test_empty_input_disables_default_extrapolator(self):
        alg = self.algorithm(EmptyInput=True)
        self.assertTrue(alg.EmptyInput)
        self.assertEqual(alg.ExtrapolatorTool.getName(), "")

    def test_extrapolator_property_is_public(self):
        self.assertIsInstance(VP1Alg.getDefaultProperty("ExtrapolatorTool"), PublicToolHandle)

    def test_event_input_retains_default_extrapolator(self):
        alg = self.algorithm(EmptyInput=False)
        self.assertEqual(alg.ExtrapolatorTool.getType(), "Trk::Extrapolator")
        self.assertEqual(alg.ExtrapolatorTool.getName(), "AtlasExtrapolator")

    def test_explicit_extrapolator_is_preserved(self):
        for empty_input in (False, True):
            with self.subTest(empty_input=empty_input):
                alg = self.algorithm(
                    EmptyInput=empty_input,
                    ExtrapolatorTool="Trk::Extrapolator/CustomExtrapolator",
                )
                self.assertEqual(alg.ExtrapolatorTool.getName(), "CustomExtrapolator")

    def test_explicitly_disabled_extrapolator_is_preserved(self):
        alg = self.algorithm(EmptyInput=False, ExtrapolatorTool="")
        self.assertEqual(alg.ExtrapolatorTool.getName(), "")

    def test_pool_event_info_conversion_uses_typed_metadata(self):
        cases = [
            (["EventInfo#EventInfo"], "EventInfo"),
            (["EventInfo#ByteStreamEventInfo"], "ByteStreamEventInfo"),
            (["PileUpEventInfo#McEventInfo"], "McEventInfo"),
            (["EventInfo#CustomEventInfo"], "CustomEventInfo"),
            (["xAOD::EventInfo#EventInfo", "EventInfo#ByteStreamEventInfo"], None),
            ([], None),
        ]
        for collections, key in cases:
            with self.subTest(collections=collections):
                flags = self.flags.clone()
                flags.Input.TypedCollections = collections
                flags.lock()
                with patch("xAODEventInfoCnv.xAODEventInfoCnvConfig.EventInfoCnvAlgCfg",
                           return_value=ComponentAccumulator()) as convert:
                    cfg = VP1EventInfoCfg(flags)
                    cfg.wasMerged()
                    if key is None:
                        convert.assert_not_called()
                    else:
                        convert.assert_called_once_with(flags, inputKey=key)

    def check_empty_input_launch(self, *options):
        cfg = ComponentAccumulator()
        self.addCleanup(cfg.wasMerged)
        # Exercise the real command-line parsing and VP1 configuration. Only
        # geometry/services and event execution are replaced to keep this a unit test.
        with patch("sys.argv", ["vp1", "--verboseAthena", *options]), \
             patch("AthenaConfiguration.MainServicesConfig.MainServicesCfg", return_value=cfg), \
             patch("VP1Algs.VP1AlgConfig.configureGeometry"), \
             patch("AthenaConfiguration.AthConfigFlags.AthConfigFlags.dump"), \
             patch.object(ComponentAccumulator, "run") as run:
            SetupVP1()
        run.assert_called_once()
        alg = cfg.getEventAlgo("VP1EventDisplayAlg")
        self.assertTrue(alg.EmptyInput)
        self.assertEqual(alg.ExtrapolatorTool.getName(), "")

    def test_standard_empty_input_launch(self):
        self.check_empty_input_launch()

    def test_remote_launch_and_override(self):
        with patch.dict(os.environ, {"SSH_CONNECTION": "client server"}):
            self.check_empty_input_launch()
            self.assertEqual(os.environ["VP1_REMOTE_MODE"], "1")
            self.check_empty_input_launch("--no-remote")
            self.assertEqual(os.environ["VP1_REMOTE_MODE"], "0")
            self.assertEqual(os.environ["VP1_REMOTE_REASON"], "")

    def test_forced_remote_launch(self):
        with patch.dict(os.environ, {"DISPLAY": ":0", "SSH_CONNECTION": "", "SSH_CLIENT": ""}), \
                patch("VP1Algs.VP1RemoteMode.remoteSessionReasons") as detect:
            self.check_empty_input_launch("--remote")
            detect.assert_not_called()
            self.assertEqual(os.environ["VP1_REMOTE_MODE"], "1")
            self.assertEqual(os.environ["VP1_REMOTE_REASON"], "requested with --remote")

    def test_conflicting_remote_flags(self):
        with patch("sys.argv", ["vp1", "--remote", "--no-remote"]), \
                contextlib.redirect_stderr(io.StringIO()), \
                self.assertRaises(SystemExit) as result:
            SetupVP1()
        self.assertEqual(result.exception.code, 2)

    def test_snapshot_opt_out_retains_remote_mode(self):
        with patch.dict(os.environ):
            self.check_empty_input_launch("--remote", "--no-fix-snapshots")
            self.assertEqual(os.environ["VP1_REMOTE_MODE"], "1")

    def test_conflicting_snapshot_flags(self):
        with patch("sys.argv", ["vp1", "--fix-snapshots", "--no-fix-snapshots"]), \
                contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as result:
            SetupVP1()
        self.assertEqual(result.exception.code, 2)

    def test_phaseII_empty_input_launch(self):
        self.check_empty_input_launch("--phaseII")


if __name__ == "__main__":
    unittest.main()
