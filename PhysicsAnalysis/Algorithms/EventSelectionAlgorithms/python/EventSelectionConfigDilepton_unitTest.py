#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Unit tests for dilepton/charge selectors: MLL, MLLWINDOW, MLL_OSSF, OS, SS,
including reco-vs-truth container routing."""

import unittest
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import run, named, prop

ELMU = {"electrons": "AnaElectrons", "muons": "AnaMuons"}


class TestMLL(unittest.TestCase):
    def test_mll(self):
        algs = named(run("MLL > 80000", containers=ELMU), "MLL_")
        self.assertEqual(len(algs), 1)
        self.assertEqual(algs[0].getType(), "CP::DileptonInvariantMassSelectorAlg")
        self.assertEqual(prop(algs[0], "sign"), "GT")
        self.assertEqual(prop(algs[0], "refMLL"), 80000.0)

    def test_missing_input(self):
        self.assertRaises(ValueError, run, "MLL > 80000")


class TestMLLWindow(unittest.TestCase):
    def test_window(self):
        algs = named(run("MLLWINDOW 80000 100000", containers=ELMU), "MLLWINDOW")
        self.assertEqual(algs[0].getType(), "CP::DileptonInvariantMassWindowSelectorAlg")
        self.assertEqual(prop(algs[0], "lowMLL"), 80000.0)
        self.assertEqual(prop(algs[0], "highMLL"), 100000.0)
        self.assertFalse(prop(algs[0], "vetoMode", False))

    def test_veto(self):
        algs = named(run("MLLWINDOW 80000 100000 veto", containers=ELMU), "MLLWINDOW")
        self.assertTrue(prop(algs[0], "vetoMode"))


class TestMLLOSSF(unittest.TestCase):
    def test_ossf(self):
        algs = named(run("MLL_OSSF 80000 100000", containers=ELMU), "MLL_OSSF")
        self.assertEqual(algs[0].getType(), "CP::DileptonOSSFInvariantMassWindowSelectorAlg")
        self.assertEqual(prop(algs[0], "lowMll"), 80000.0)
        self.assertEqual(prop(algs[0], "highMll"), 100000.0)
        self.assertFalse(prop(algs[0], "vetoMode", False))

    def test_veto(self):
        algs = named(run("MLL_OSSF 80000 100000 veto", containers=ELMU), "MLL_OSSF")
        self.assertTrue(prop(algs[0], "vetoMode"))

    def test_truth_routing(self):
        algs = named(run("MLL_OSSF 80000 100000",
                         containers={"electrons": "AnaTruthElectrons", "muons": "AnaTruthMuons"}), "MLL_OSSF")
        # truth containers must populate the truth* handles, not the reco ones
        self.assertNotEqual(prop(algs[0], "truthElectrons", ""), "")
        self.assertNotEqual(prop(algs[0], "truthMuons", ""), "")


class TestCharge(unittest.TestCase):
    def test_os_ss(self):
        for kw, osmode in [("OS", True), ("SS", False)]:
            with self.subTest(kw=kw):
                algs = named(run(kw, containers=ELMU), f"_{kw}_")
                self.assertEqual(algs[0].getType(), "CP::ChargeSelectorAlg")
                self.assertEqual(prop(algs[0], "OS"), osmode)

    def test_truth_routing(self):
        algs = named(run("OS", containers={"electrons": "AnaTruthElectrons"}), "_OS_")
        self.assertNotEqual(prop(algs[0], "truthElectrons", ""), "")

    def test_missing_input(self):
        self.assertRaises(ValueError, run, "OS")

    def test_subset_selector(self):
        # "OS el" restricts the charge computation to electrons only
        algs = named(run("OS el", containers=ELMU), "_OS_")
        self.assertNotEqual(prop(algs[0], "electrons", ""), "")
        self.assertEqual(prop(algs[0], "muons", ""), "")


if __name__ == "__main__":
    unittest.main()
