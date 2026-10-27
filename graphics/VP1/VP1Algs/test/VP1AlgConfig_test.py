# Copyright (C) 2026 CERN for the benefit of the ATLAS collaboration

"""Regression tests for VP1 extrapolator configuration, without a GUI or geometry."""

import unittest
from unittest.mock import patch

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from VP1Algs.VP1AlgConfig import SetupVP1, VP1AlgCfg


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

    def test_phaseII_empty_input_launch(self):
        self.check_empty_input_launch("--phaseII")


if __name__ == "__main__":
    unittest.main()
