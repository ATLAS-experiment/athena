#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Unit tests for SUM_EL_N_MU_N and SUM_EL_N_MU_N_TAU_N (SumNLeptonPtSelectorAlg)."""

import unittest
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import run, named, prop

ELMU = {"electrons": "AnaElectrons", "muons": "AnaMuons"}
ELMUTAU = {"electrons": "AnaElectrons", "muons": "AnaMuons", "taus": "AnaTaus"}


class TestSumElMu(unittest.TestCase):
    def test_shared_threshold_4arg(self):
        algs = named(run("SUM_EL_N_MU_N 25000 >= 2", containers=ELMU), "SUMNELNMU")
        self.assertEqual(len(algs), 1)
        self.assertEqual(algs[0].getType(), "CP::SumNLeptonPtSelectorAlg")
        self.assertEqual(prop(algs[0], "minPtEl"), 25000.0)
        self.assertEqual(prop(algs[0], "minPtMu"), 25000.0)
        self.assertEqual(prop(algs[0], "count"), 2)

    def test_separate_thresholds_5arg(self):
        algs = named(run("SUM_EL_N_MU_N 25000 27000 >= 2", containers=ELMU), "SUMNELNMU")
        self.assertEqual(prop(algs[0], "minPtEl"), 25000.0)
        self.assertEqual(prop(algs[0], "minPtMu"), 27000.0)

    def test_extra_selections_7arg(self):
        algs = named(run("SUM_EL_N_MU_N elsel musel 25000 27000 >= 2", containers=ELMU), "SUMNELNMU")
        self.assertIn("elsel", prop(algs[0], "electronSelection"))
        self.assertIn("musel", prop(algs[0], "muonSelection"))
        self.assertEqual(prop(algs[0], "count"), 2)

    def test_missing_input(self):
        self.assertRaises(ValueError, run, "SUM_EL_N_MU_N 25000 >= 2")

    def test_bad_argcount(self):
        self.assertRaises(ValueError, run, "SUM_EL_N_MU_N 25000 27000 28000 >= 2", containers=ELMU)


class TestSumElMuTau(unittest.TestCase):
    def test_shared_threshold_4arg(self):
        algs = named(run("SUM_EL_N_MU_N_TAU_N 25000 >= 3", containers=ELMUTAU), "SUMNLEPTONS")
        self.assertEqual(prop(algs[0], "minPtEl"), 25000.0)
        self.assertEqual(prop(algs[0], "minPtMu"), 25000.0)
        self.assertEqual(prop(algs[0], "minPtTau"), 25000.0)
        self.assertEqual(prop(algs[0], "count"), 3)

    def test_separate_thresholds_6arg(self):
        algs = named(run("SUM_EL_N_MU_N_TAU_N 25000 27000 30000 >= 3", containers=ELMUTAU), "SUMNLEPTONS")
        self.assertEqual(prop(algs[0], "minPtEl"), 25000.0)
        self.assertEqual(prop(algs[0], "minPtMu"), 27000.0)
        self.assertEqual(prop(algs[0], "minPtTau"), 30000.0)

    def test_extra_selections_9arg(self):
        algs = named(run("SUM_EL_N_MU_N_TAU_N esel msel tsel 25000 27000 30000 >= 3",
                         containers=ELMUTAU), "SUMNLEPTONS")
        self.assertIn("esel", prop(algs[0], "electronSelection"))
        self.assertIn("msel", prop(algs[0], "muonSelection"))
        self.assertIn("tsel", prop(algs[0], "tauSelection"))

    def test_bad_argcount(self):
        self.assertRaises(ValueError, run, "SUM_EL_N_MU_N_TAU_N 25000 27000 >= 3", containers=ELMUTAU)


if __name__ == "__main__":
    unittest.main()
