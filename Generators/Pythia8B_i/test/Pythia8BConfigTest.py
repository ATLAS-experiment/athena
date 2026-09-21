#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Unit tests for the Pythia8B ComponentAccumulator configuration."""

from functools import partial
import unittest
from unittest.mock import patch

from AthenaCommon.SystemOfUnits import GeV
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from GeneratorConfig.GeneratorSettingsSemantics import (
    GeneratorSettingsPrecedence,
)
from Pythia8B_i import Pythia8BConfig as config
from Pythia8B_i import Pythia8BProcesses as processes


class TestPythia8BConfig(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.flags = initConfigFlags()
        cls.flags.Beam.Energy = 6800 * GeV
        cls.flags.Random.SeedOffset = 17
        cls.flags.Generator.DSID = 999999
        cls.flags.lock()

    def configure(self, factory, **kwargs):
        ca = factory(self.flags, **kwargs)
        self.addCleanup(ca.wasMerged)
        return ca

    def test_base_settings(self):
        ca = self.configure(config.Pythia8BBaseCfg, name="CustomPythia8B")
        alg = ca.getEventAlgo("CustomPythia8B")
        self.assertEqual(alg.CollisionEnergy, 13600)
        self.assertEqual(alg.RandomSeed, 17)
        self.assertEqual(alg.Dsid, 999999)
        self.assertEqual(
            list(ca.getService("GeneratorInfoSvc").Generators), ["Pythia8B"])

    def test_process_and_user_command_precedence(self):
        ca = self.configure(
            processes.Pythia8B_exclusiveB_Common_Cfg,
            ShowerCfg=config.Pythia8B_A14_CTEQ6L1_Common_Cfg,
            Commands=["HardQCD:all = off"],
        )
        alg = ca.getEventAlgo("Pythia8B")
        self.assertEqual(alg.Commands.data[-1], "HardQCD:all = off")
        self.assertEqual(
            [layer.precedence for layer in alg.Commands.layers],
            [GeneratorSettingsPrecedence.BASE,
             GeneratorSettingsPrecedence.TUNE,
             GeneratorSettingsPrecedence.MATCHING,
             GeneratorSettingsPrecedence.USER],
        )

    def test_processes_and_tunes(self):
        process_cfgs = (
            processes.Pythia8B_exclusiveB_Common_Cfg,
            processes.Pythia8B_exclusiveAntiB_Common_Cfg,
            processes.Pythia8B_inclusiveBJpsi_Common_Cfg,
            processes.Pythia8B_inclusiveAntiBJpsi_Common_Cfg,
            processes.Pythia8B_Charmonium_Common_Cfg,
            processes.Pythia8B_Bottomonium_Common_Cfg,
        )
        tune_cfgs = (
            (config.Pythia8B_A14_CTEQ6L1_Common_Cfg, "A14 CTEQ6L1"),
            (config.Pythia8B_A14_NNPDF23LO_Common_Cfg, "A14 NNPDF23LO"),
        )
        for process_cfg in process_cfgs:
            for tune_cfg, tune in tune_cfgs:
                with self.subTest(process=process_cfg.__name__, tune=tune):
                    ca = self.configure(process_cfg, ShowerCfg=tune_cfg)
                    self.assertEqual(len(ca.getEventAlgos()), 1)
                    self.assertEqual(ca.getService("GeneratorInfoSvc").Tune,
                                     tune)

    def test_nested_decay_fragments_share_one_layer(self):
        shower_cfg = partial(
            processes.Pythia8B_exclusiveB_Common_Cfg,
            ShowerCfg=config.Pythia8B_A14_CTEQ6L1_Common_Cfg,
        )
        ca = self.configure(
            processes.Pythia8B_OpenBJpsiDecays_Cfg,
            ShowerCfg=shower_cfg,
        )
        alg = ca.getEventAlgo("Pythia8B")
        matching_layers = [
            layer for layer in alg.Commands.layers
            if layer.precedence == GeneratorSettingsPrecedence.MATCHING
        ]
        self.assertEqual(len(matching_layers), 1)
        self.assertIn(
            "511:onIfAny = 443 100443 445 10441 10443 20443",
            alg.Commands.data,
        )

    @patch("PyJobTransformsCore.trfutil.get_files")
    def test_evtgen_and_photos(self, get_files):
        ca = self.configure(
            config.Pythia8B_Photospp_Cfg,
            ShowerCfg=config.Pythia8B_A14_CTEQ6L1_EvtGen_Common_Cfg,
            EvtGenOptions={"userDecayFile": "exclusive.dec"},
        )
        self.assertEqual(
            [alg.name for alg in ca.getSequence("EvgenGenSeq").Members],
            ["Pythia8B", "EvtInclusiveDecay", "Photospp_i"],
        )
        evtgen = ca.getEventAlgo("EvtInclusiveDecay")
        self.assertEqual(evtgen.pdtFile, "inclusiveP8DsDPlus.pdt")
        self.assertEqual(evtgen.userDecayFile, "exclusive.dec")
        self.assertIn("inclusiveP8DsDPlus.pdt", get_files.call_args.args[0])
        self.assertIn(
            "TimeShower:QEDshowerByL = off",
            ca.getEventAlgo("Pythia8B").Commands.data,
        )


if __name__ == "__main__":
    unittest.main()
