#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Test the configuration used by the migrated DSID 801918 job option."""

from functools import partial
import unittest

from AthenaCommon.SystemOfUnits import GeV
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from GeneratorConfig.GeneratorSettingsSemantics import (
    GeneratorSettingsPrecedence,
)
from Pythia8B_i.Pythia8BConfig import (
    Pythia8B_A14_CTEQ6L1_Common_Cfg,
    Pythia8B_exclusiveB_Common_Cfg,
    Pythia8B_Photospp_Cfg,
)


class TestPythia8BConfig(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.flags = initConfigFlags()
        cls.flags.Beam.Energy = 6800 * GeV
        cls.flags.Random.SeedOffset = 17
        cls.flags.Generator.DSID = 801918
        cls.flags.lock()

    def test_dsid_801918_configuration(self):
        shower_cfg = partial(
            Pythia8B_Photospp_Cfg,
            ShowerCfg=Pythia8B_A14_CTEQ6L1_Common_Cfg,
        )
        ca = Pythia8B_exclusiveB_Common_Cfg(
            self.flags,
            ShowerCfg=shower_cfg,
            Commands=[
                "PhaseSpace:pTHatMin = 7.",
                "531:addChannel = 2 1.0 0 -13 13",
            ],
            QuarkPtCut=0.0,
            AntiQuarkPtCut=7.0,
            QuarkEtaCut=102.5,
            AntiQuarkEtaCut=2.6,
            RequireBothQuarksPassCuts=True,
            VetoDoubleBEvents=True,
            SignalPDGCodes=[531, -13, 13],
            NHadronizationLoops=2,
            TriggerPDGCode=13,
            TriggerStatePtCut=[3.5],
            TriggerStateEtaCut=2.6,
            MinimumCountPerCut=[2],
        )
        self.addCleanup(ca.wasMerged)

        alg = ca.getEventAlgo("Pythia8B")
        self.assertEqual(alg.CollisionEnergy, 13600)
        self.assertEqual(alg.Dsid, 801918)
        self.assertEqual(alg.NHadronizationLoops, 2)
        self.assertEqual(list(alg.SignalPDGCodes), [531, -13, 13])
        self.assertEqual(alg.AntiQuarkPtCut, 7.0)
        self.assertEqual(alg.AntiQuarkEtaCut, 2.6)
        self.assertTrue(alg.RequireBothQuarksPassCuts)
        self.assertEqual(list(alg.TriggerStatePtCut), [3.5])
        self.assertEqual(list(alg.MinimumCountPerCut), [2])
        self.assertEqual(
            sorted(layer.precedence for layer in alg.Commands.layers),
            [GeneratorSettingsPrecedence.BASE,
             GeneratorSettingsPrecedence.TUNE,
             GeneratorSettingsPrecedence.MATCHING,
             GeneratorSettingsPrecedence.USER],
        )
        self.assertIn("TimeShower:QEDshowerByL = off", alg.Commands.data)
        self.assertEqual(
            [member.name for member in ca.getSequence("EvgenGenSeq").Members],
            ["Pythia8B", "Photospp_i"],
        )


if __name__ == "__main__":
    unittest.main()
