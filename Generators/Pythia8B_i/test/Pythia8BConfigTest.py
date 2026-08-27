#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import unittest

from AthenaCommon.SystemOfUnits import GeV
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from GeneratorConfig.GeneratorSettingsSemantics import (
    GeneratorSettingsPrecedence,
)
from Pythia8B_i.Pythia8BConfig import (
    Pythia8B_A14_CTEQ6L1_Common_Cfg,
    Pythia8B_exclusiveB_Common_Cfg,
)


class TestPythia8BConfig(unittest.TestCase):
    """Test composition and precedence of the Pythia8B CA fragments."""

    @classmethod
    def setUpClass(cls):
        cls.flags = initConfigFlags()
        cls.flags.Beam.Energy = 6800 * GeV
        cls.flags.Random.SeedOffset = 17
        cls.flags.Generator.DSID = 999999
        cls.flags.lock()

    def test_exclusive_b_configuration(self):
        ca = Pythia8B_exclusiveB_Common_Cfg(
            self.flags,
            ShowerCfg=Pythia8B_A14_CTEQ6L1_Common_Cfg,
            Commands=(
                "HardQCD:all = off",
                "HardQCD:gg2bbbar = on",
            ),
            NHadronizationLoops=10,
        )
        pythia8b = ca.getEventAlgo("Pythia8B")

        self.assertEqual(pythia8b.CollisionEnergy, 13600)
        self.assertEqual(pythia8b.RandomSeed, 17)
        self.assertEqual(pythia8b.Dsid, 999999)
        self.assertTrue(pythia8b.SelectBQuarks)
        self.assertFalse(pythia8b.SelectCQuarks)
        self.assertEqual(pythia8b.NHadronizationLoops, 10)

        self.assertEqual(
            [layer.precedence for layer in pythia8b.Commands.layers],
            [
                GeneratorSettingsPrecedence.BASE,
                GeneratorSettingsPrecedence.TUNE,
                GeneratorSettingsPrecedence.PROCESS,
                GeneratorSettingsPrecedence.USER,
            ],
        )
        self.assertIn(
            "PDF:pSet = LHAPDF6:cteq6l1",
            pythia8b.Commands.data,
        )
        self.assertIn("HardQCD:all = off", pythia8b.Commands.data)
        self.assertIn("HardQCD:gg2bbbar = on", pythia8b.Commands.data)
        self.assertNotIn("HardQCD:all = on", pythia8b.Commands.data)


if __name__ == "__main__":
    unittest.main()
