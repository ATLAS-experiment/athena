#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Check the migrated job option and Pythia8B fragment composition."""

from functools import partial
from pathlib import Path
import runpy
import unittest

from AthenaCommon.SystemOfUnits import GeV
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from Pythia8B_i.Pythia8BConfig import (
    Pythia8B_A14_CTEQ6L1_Common_Cfg,
    Pythia8B_exclusiveB_Common_Cfg,
    Pythia8B_Photospp_Cfg,
)


class TestPythia8BConfig(unittest.TestCase):
    def setUp(self):
        self.flags = initConfigFlags()
        self.flags.Beam.Energy = 6800 * GeV
        self.flags.Random.SeedOffset = 17
        self.flags.Generator.DSID = 801918

    def test_dsid_801918_configuration(self):
        # CTest runs this script from the source tree. Load the actual job
        # option so changes to it are covered without keeping a second copy.
        job_option = (
            Path(__file__).resolve().parents[2]
            / "EvgenJobTransforms/share/EvgenTest/801918"
            / "mc.P8B_A14_CTEQ6L1_Bs_m3p5mu3p5.py"
        )
        sample = runpy.run_path(str(job_option))["Sample"](self.flags)
        sample.setupFlags(self.flags)
        self.flags.lock()
        ca = sample.setupProcess(self.flags)
        self.addCleanup(ca.wasMerged)

        self.assertEqual(sample.process, "Bs -> mumu")
        self.assertEqual(sample.nEventsPerJob, 500)
        info = ca.getService("GeneratorInfoSvc")
        self.assertEqual(list(info.Generators), ["Pythia8B", "Photospp"])
        self.assertEqual(info.Tune, "A14 CTEQ6L1")

        alg = ca.getEventAlgo("Pythia8B")
        self.assertEqual(alg.CollisionEnergy, 13600)
        self.assertEqual(alg.RandomSeed, 17)
        self.assertEqual(alg.Dsid, 801918)
        self.assertEqual(alg.NHadronizationLoops, 2)
        self.assertEqual(list(alg.SignalPDGCodes), [531, -13, 13])
        self.assertTrue(alg.SelectBQuarks)
        self.assertFalse(alg.SelectCQuarks)
        self.assertTrue(alg.VetoDoubleBEvents)
        self.assertEqual(
            list(alg.BPDGCodes),
            [511, 521, 531, 541, 5122, 5132, 5232, 5332,
             -511, -521, -531, -541, -5122, -5132, -5232, -5332],
        )
        self.assertEqual(alg.QuarkPtCut, 0.0)
        self.assertEqual(alg.AntiQuarkPtCut, 7.0)
        self.assertEqual(alg.QuarkEtaCut, 102.5)
        self.assertEqual(alg.AntiQuarkEtaCut, 2.6)
        self.assertTrue(alg.RequireBothQuarksPassCuts)
        self.assertEqual(alg.TriggerPDGCode, 13)
        self.assertEqual(list(alg.TriggerStatePtCut), [3.5])
        self.assertEqual(alg.TriggerStateEtaCut, 2.6)
        self.assertEqual(list(alg.MinimumCountPerCut), [2])

        commands = alg.Commands.data
        for command in (
            "PDF:pSet = LHAPDF6:cteq6l1",
            "SpaceShower:rapidityOrderMPI = on",
            "HardQCD:all = on",
            "ParticleDecays:mixB = off",
            "HadronLevel:all = off",
            "TimeShower:QEDshowerByL = off",
            "PhaseSpace:pTHatMin = 7.",
            "531:addChannel = 2 1.0 0 -13 13",
        ):
            with self.subTest(command=command):
                self.assertEqual(commands.count(command), 1)
        # Close the original B decays before adding the signal decay channel.
        self.assertEqual(
            [cmd for cmd in commands
             if ":onMode" in cmd or ":addChannel" in cmd],
            ["511:onMode = 3", "521:onMode = 3", "531:onMode = 3",
             "541:onMode = 3", "5122:onMode = 2", "5132:onMode = 2",
             "5232:onMode = 2", "5332:onMode = 2",
             "531:addChannel = 2 1.0 0 -13 13"],
        )
        self.assertEqual(
            [alg.name for alg in ca.getEventAlgos("EvgenGenSeq")],
            ["Pythia8B", "Photospp_i"],
        )
        self.assertEqual(ca.getEventAlgo("Photospp_i").InfraRedCutOff, 1E-7)

    def test_custom_name_and_user_overrides(self):
        self.flags.lock()
        ca = Pythia8B_exclusiveB_Common_Cfg(
            self.flags,
            ShowerCfg=partial(
                Pythia8B_Photospp_Cfg,
                ShowerCfg=Pythia8B_A14_CTEQ6L1_Common_Cfg,
            ),
            name="CustomPythia8B",
            CollisionEnergy=13000,
            RandomSeed=23,
            Dsid=999999,
            VetoDoubleBEvents=False,
            Commands=[
                "Main:timesAllowErrors = 1000",  # Override the base.
                "SpaceShower:alphaSvalue = 0.130",  # Override the tune.
                "ParticleDecays:mixB = on",  # Override the process.
            ],
        )
        self.addCleanup(ca.wasMerged)

        self.assertEqual(
            [alg.name for alg in ca.getEventAlgos("EvgenGenSeq")],
            ["CustomPythia8B", "Photospp_i"],
        )
        alg = ca.getEventAlgo("CustomPythia8B")
        self.assertEqual(alg.CollisionEnergy, 13000)
        self.assertEqual(alg.RandomSeed, 23)
        self.assertEqual(alg.Dsid, 999999)
        self.assertFalse(alg.VetoDoubleBEvents)
        for key, value in (
            ("Main:timesAllowErrors", "1000"),
            ("SpaceShower:alphaSvalue", "0.130"),
            ("ParticleDecays:mixB", "on"),
        ):
            with self.subTest(key=key):
                self.assertEqual(
                    [cmd for cmd in alg.Commands.data
                     if cmd.split("=", 1)[0].strip() == key],
                    [f"{key} = {value}"],
                )


if __name__ == "__main__":
    unittest.main()
