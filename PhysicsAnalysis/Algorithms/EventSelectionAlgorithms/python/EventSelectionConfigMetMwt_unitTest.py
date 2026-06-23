#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Unit tests for MET, MWT and MET+MWT selectors."""

import unittest
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import run, named, prop


class TestMET(unittest.TestCase):
    def test_met(self):
        algs = named(run("MET > 30000", containers={"met": "AnaMET"}, metTerm="Final"), "MET")
        self.assertEqual(len(algs), 1)
        self.assertEqual(algs[0].getType(), "CP::MissingETSelectorAlg")
        self.assertEqual(prop(algs[0], "sign"), "GT")
        self.assertEqual(prop(algs[0], "refMET"), 30000.0)
        self.assertEqual(prop(algs[0], "metTerm"), "Final")

    def test_custom_term(self):
        algs = named(run("MET >= 50000", containers={"met": "AnaMET"}, metTerm="NonInt"), "MET")
        self.assertEqual(prop(algs[0], "metTerm"), "NonInt")

    def test_missing_input(self):
        self.assertRaises(ValueError, run, "MET > 30000")

    def test_bad_argcount(self):
        self.assertRaises(ValueError, run, "MET > 30000 50000", containers={"met": "AnaMET"})


class TestMWT(unittest.TestCase):
    def test_mwt(self):
        algs = named(run("MWT > 50000",
                         containers={"met": "AnaMET", "electrons": "AnaElectrons", "muons": "AnaMuons"}), "MWT")
        self.assertEqual(algs[0].getType(), "CP::TransverseMassSelectorAlg")
        self.assertEqual(prop(algs[0], "sign"), "GT")
        self.assertEqual(prop(algs[0], "refMWT"), 50000.0)

    def test_missing_leptons(self):
        self.assertRaises(ValueError, run, "MWT > 50000", containers={"met": "AnaMET"})


class TestMETMWT(unittest.TestCase):
    def test_metmwt(self):
        algs = named(run("MET+MWT > 60000",
                         containers={"met": "AnaMET", "electrons": "AnaElectrons", "muons": "AnaMuons"}), "METMWT")
        self.assertEqual(algs[0].getType(), "CP::MissingETPlusTransverseMassSelectorAlg")
        self.assertEqual(prop(algs[0], "sign"), "GT")
        self.assertEqual(prop(algs[0], "refMETMWT"), 60000.0)

    def test_missing_met(self):
        self.assertRaises(ValueError, run, "MET+MWT > 60000", containers={"electrons": "AnaElectrons"})

    def test_missing_leptons(self):
        self.assertRaises(ValueError, run, "MET+MWT > 60000", containers={"met": "AnaMET"})


if __name__ == "__main__":
    unittest.main()
