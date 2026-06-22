#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Unit tests for LJETMASS_N (NObjectMassSelectorAlg) and LJETMASSWINDOW_N
(NLargeRJetMassWindowSelectorAlg), including veto-mode parsing."""

import unittest
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import run, named, prop

LRJ = {"largeRjets": "AnaLRJets"}


class TestLJetMass(unittest.TestCase):
    def test_minimal(self):
        algs = named(run("LJETMASS_N 50000 >= 1", containers=LRJ), "NLJETMASS")
        self.assertEqual(len(algs), 1)
        self.assertEqual(algs[0].getType(), "CP::NObjectMassSelectorAlg")
        self.assertEqual(prop(algs[0], "minMass"), 50000.0)
        self.assertEqual(prop(algs[0], "sign"), "GE")
        self.assertEqual(prop(algs[0], "count"), 1)

    def test_extra_selection(self):
        algs = named(run("LJETMASS_N topjet 50000 >= 1", containers=LRJ), "NLJETMASS")
        self.assertIn("topjet", prop(algs[0], "objectSelection"))


class TestLJetMassWindow(unittest.TestCase):
    def test_minimal(self):
        algs = named(run("LJETMASSWINDOW_N 60000 100000 >= 1", containers=LRJ), "NLJETMASSWINDOW")
        self.assertEqual(len(algs), 1)
        self.assertEqual(algs[0].getType(), "CP::NLargeRJetMassWindowSelectorAlg")
        self.assertEqual(prop(algs[0], "lowMass"), 60000.0)
        self.assertEqual(prop(algs[0], "highMass"), 100000.0)
        self.assertEqual(prop(algs[0], "count"), 1)
        self.assertFalse(prop(algs[0], "vetoMode", False))

    def test_veto(self):
        algs = named(run("LJETMASSWINDOW_N 60000 100000 >= 1 veto", containers=LRJ), "NLJETMASSWINDOW")
        self.assertTrue(prop(algs[0], "vetoMode"))

    def test_extra_selection(self):
        algs = named(run("LJETMASSWINDOW_N topjet 60000 100000 >= 1", containers=LRJ), "NLJETMASSWINDOW")
        self.assertIn("topjet", prop(algs[0], "ljetSelection"))
        self.assertEqual(prop(algs[0], "lowMass"), 60000.0)

    def test_extra_selection_and_veto(self):
        algs = named(run("LJETMASSWINDOW_N topjet 60000 100000 >= 1 veto", containers=LRJ), "NLJETMASSWINDOW")
        self.assertIn("topjet", prop(algs[0], "ljetSelection"))
        self.assertTrue(prop(algs[0], "vetoMode"))

    def test_wrong_argcount(self):
        self.assertRaises(ValueError, run, "LJETMASSWINDOW_N 60000 >= 1", containers=LRJ)


if __name__ == "__main__":
    unittest.main()
