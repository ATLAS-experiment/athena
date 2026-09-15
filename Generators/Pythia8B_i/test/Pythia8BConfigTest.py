#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Configuration tests; event generation is covered by the lxplus smoke jobs."""

from functools import partial
import tempfile
from pathlib import Path
import unittest
from unittest.mock import patch

from AthenaCommon.SystemOfUnits import GeV
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from GeneratorConfig.GeneratorSettingsSemantics import GeneratorSettingsPrecedence
from Pythia8B_i import Pythia8BConfig as config


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

    def test_exclusive_b_overrides_and_custom_name(self):
        commands = ("HardQCD:all = off", "HardQCD:gg2bbbar = on")
        ca = self.configure(
            config.Pythia8B_exclusiveB_Common_Cfg,
            ShowerCfg=config.Pythia8B_A14_CTEQ6L1_Common_Cfg,
            name="CustomPythia8B", Commands=commands, NHadronizationLoops=10,
        )
        alg = ca.getEventAlgo("CustomPythia8B")
        self.assertEqual(alg.CollisionEnergy, 13600)
        self.assertEqual(alg.RandomSeed, 17)
        self.assertEqual(alg.Dsid, 999999)
        self.assertTrue(alg.SelectBQuarks)
        self.assertFalse(alg.SelectCQuarks)
        self.assertEqual(alg.NHadronizationLoops, 10)
        self.assertEqual(alg.Commands.data[-2:], list(commands))
        self.assertLess(alg.Commands.data.index("HardQCD:all = on"),
                        alg.Commands.data.index("HardQCD:all = off"))
        self.assertEqual(
            sorted(layer.precedence for layer in alg.Commands.layers),
            [GeneratorSettingsPrecedence.BASE, GeneratorSettingsPrecedence.TUNE,
             GeneratorSettingsPrecedence.PROCESS, GeneratorSettingsPrecedence.USER],
        )
        self.assertEqual(list(ca.getService("GeneratorInfoSvc").Generators), ["Pythia8B"])

    def test_all_processes_and_tunes(self):
        processes = (
            config.Pythia8B_exclusiveB_Common_Cfg,
            config.Pythia8B_exclusiveAntiB_Common_Cfg,
            config.Pythia8B_inclusiveBJpsi_Common_Cfg,
            config.Pythia8B_inclusiveAntiBJpsi_Common_Cfg,
            config.Pythia8B_Charmonium_Common_Cfg,
            config.Pythia8B_Bottomonium_Common_Cfg,
        )
        tunes = (
            (config.Pythia8B_A14_CTEQ6L1_Common_Cfg, "A14 CTEQ6L1", "cteq6l1"),
            (config.Pythia8B_A14_NNPDF23LO_Common_Cfg, "A14 NNPDF23LO",
             "NNPDF23_lo_as_0130_qed"),
        )
        for process in processes:
            for tune, label, pdf in tunes:
                with self.subTest(process=process.__name__, tune=label):
                    ca = self.configure(process, ShowerCfg=tune)
                    self.assertEqual(len(ca.getEventAlgos()), 1)
                    alg = ca.getEventAlgo("Pythia8B")
                    self.assertIn("PDF:pSet = LHAPDF6:" + pdf, alg.Commands.data)
                    self.assertIn("SpaceShower:rapidityOrderMPI = on", alg.Commands.data)
                    self.assertEqual(ca.getService("GeneratorInfoSvc").Tune, label)
                    if "onium" in process.__name__:
                        self.assertTrue(alg.SuppressSmallPT)
                        self.assertFalse(alg.SelectBQuarks)
                        self.assertFalse(alg.VetoDoubleCEvents)
                    else:
                        self.assertTrue(alg.SelectBQuarks)
                        self.assertEqual(len(alg.BPDGCodes), 16)
                    if "inclusive" in process.__name__:
                        self.assertEqual(alg.UserSelection, "BJPSIINCLUSIVE")
                    if "inclusiveAnti" in process.__name__:
                        self.assertEqual(list(alg.UserSelectionVariables), [-1])

    def test_all_charmonium_channels_survive(self):
        for factory in (config.Pythia8B_inclusiveBJpsi_Common_Cfg,
                        config.Pythia8B_inclusiveAntiBJpsi_Common_Cfg):
            with self.subTest(factory=factory.__name__):
                ca = self.configure(factory, ShowerCfg=config.Pythia8B_A14_CTEQ6L1_Common_Cfg)
                commands = ca.getEventAlgo("Pythia8B").Commands.data
                opened = [command for command in commands if ":onIfAny" in command]
                self.assertEqual(len(opened), 8)
                for command in opened:
                    self.assertEqual(command.split("=")[1].split(),
                                     ["443", "100443", "445", "10441", "10443", "20443"])

    def test_nested_decay_fragments_form_one_process_layer(self):
        shower = partial(config.Pythia8B_exclusiveB_Common_Cfg,
                         ShowerCfg=config.Pythia8B_A14_CTEQ6L1_Common_Cfg)
        ca = self.configure(config.Pythia8B_OpenBJpsiDecays_Cfg, ShowerCfg=shower)
        alg = ca.getEventAlgo("Pythia8B")
        self.assertEqual(len(ca.getEventAlgos()), 1)
        self.assertEqual(sum(layer.precedence == GeneratorSettingsPrecedence.PROCESS
                             for layer in alg.Commands.layers), 1)
        self.assertLess(alg.Commands.data.index("511:onMode = 3"),
                        alg.Commands.data.index("511:onIfAny = 443 100443 445 10441 10443 20443"))

    def test_decay_signs_match_legacy(self):
        for factory, meson, baryon in (
            (config.Pythia8B_CloseBDecays_Cfg, 3, 2),
            (config.Pythia8B_CloseAntiBDecays_Cfg, 2, 3),
        ):
            with self.subTest(factory=factory.__name__):
                ca = self.configure(factory, ShowerCfg=config.Pythia8BBaseCfg)
                commands = ca.getEventAlgo("Pythia8B").Commands.data
                self.assertIn(f"511:onMode = {meson}", commands)
                self.assertIn(f"5122:onMode = {baryon}", commands)

    def test_ordered_operations_and_idempotent_merge(self):
        commands = ("443:onMode = off", "443:onIfAny = 13",
                    "443:onMode = on", "443:onIfAny = 11",
                    "443:addChannel = 1 0.1 0 13 -13",
                    "443:addChannel = 1 0.1 0 13 -13")
        first = self.configure(config.Pythia8B_A14_CTEQ6L1_Common_Cfg, Commands=commands)
        second = self.configure(config.Pythia8B_A14_CTEQ6L1_Common_Cfg, Commands=commands)
        first.merge(second)
        self.assertEqual(first.getEventAlgo("Pythia8B").Commands.data[-len(commands):],
                         list(commands))
        self.assertEqual(len(first.getEventAlgos()), 1)

    def test_invalid_commands_fail_before_ca_construction(self):
        for commands in ("HardQCD:all = on", [42]):
            with self.subTest(commands=commands), self.assertRaises(TypeError):
                config.Pythia8BBaseCfg(self.flags, Commands=commands)

    @patch("PyJobTransformsCore.trfutil.get_files")
    def test_evtgen_and_photos_order_and_options(self, get_files):
        for tune in (config.Pythia8B_A14_CTEQ6L1_EvtGen_Common_Cfg,
                     config.Pythia8B_A14_NNPDF23LO_EvtGen_Common_Cfg):
            with self.subTest(tune=tune.__name__):
                ca = self.configure(
                    config.Pythia8B_Photospp_Cfg, ShowerCfg=tune,
                    EvtGenOptions={"userDecayFile": "exclusive.dec", "RandomSeed": 31},
                    PhotosppOptions={"InfraRedCutOff": 1e-6},
                )
                self.assertEqual([alg.name for alg in ca.getSequence("EvgenGenSeq").Members],
                                 ["Pythia8B", "EvtInclusiveDecay", "Photospp"])
                evtgen = ca.getEventAlgo("EvtInclusiveDecay")
                self.assertEqual(evtgen.pdtFile, "inclusiveP8DsDPlus.pdt")
                self.assertEqual(evtgen.userDecayFile, "exclusive.dec")
                self.assertEqual(evtgen.RandomSeed, 31)
                self.assertEqual(evtgen.Dsid, 999999)
                self.assertIn(5334, evtgen.whiteList)
                photos = ca.getEventAlgo("Photospp")
                self.assertEqual(photos.RandomSeed, 17)
                self.assertEqual(photos.Dsid, 999999)
                self.assertEqual(photos.InfraRedCutOff, 1e-6)
                self.assertIn("TimeShower:QEDshowerByL = off",
                              ca.getEventAlgo("Pythia8B").Commands.data)
                self.assertEqual(set(ca.getService("GeneratorInfoSvc").Generators),
                                 {"Pythia8B", "EvtGen", "Photospp"})
                self.assertIn("exclusive.dec", get_files.call_args.args[0])

    @patch("PyJobTransformsCore.trfutil.get_files")
    def test_evtgen_local_and_auxiliary_files(self, get_files):
        from EvtGen_i.EvtGenConfig import EvtGenCfg

        with tempfile.TemporaryDirectory() as directory:
            local = Path(directory) / "user.dec"
            local.write_text("End\n")
            ca = self.configure(EvtGenCfg, name="CustomEvtGen", userDecayFile=str(local),
                                decayFile="custom.dec", pdtFile="custom.pdt",
                                auxfiles=["extra.dat", "custom.dec", "extra.dat"],
                                whiteList=[5334, 5334])
            self.assertEqual(set(get_files.call_args.args[0]),
                             {"custom.dec", "custom.pdt", "extra.dat"})
            alg = ca.getEventAlgo("CustomEvtGen")
            self.assertEqual(alg.RandomSeed, 17)
            self.assertEqual(alg.userDecayFile, str(local))
            self.assertEqual(list(alg.whiteList).count(5334), 1)

    @patch("PyJobTransformsCore.trfutil.get_files")
    def test_standard_pythia_evtgen_regression(self, get_files):
        from Pythia8_i.Pythia8Config import Pythia8_A14_NNPDF23LO_EvtGen_Common_Cfg

        ca = self.configure(Pythia8_A14_NNPDF23LO_EvtGen_Common_Cfg,
                            Commands=["HardQCD:gg2bbbar = on"])
        evtgen = ca.getEventAlgo("EvtInclusiveDecay")
        self.assertEqual(evtgen.pdtFile, "inclusive.pdt")
        self.assertEqual(evtgen.RandomSeed, 17)
        self.assertEqual(evtgen.Dsid, 999999)
        self.assertEqual([alg.name for alg in ca.getSequence("EvgenGenSeq").Members],
                         ["Pythia8_i", "EvtInclusiveDecay"])


if __name__ == "__main__":
    unittest.main()
