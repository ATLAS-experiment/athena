#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Routing tests: each keyword reaches the right algorithm, prefix-collision
keywords are not mis-routed, and non-cut lines are skipped. These guard the
fragile token-membership dispatch against silent breakage on reordering."""

import unittest
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import run, selectors, named

ALL = dict(electrons="AnaElectrons", muons="AnaMuons", taus="AnaTaus",
           jets="AnaJets", largeRjets="AnaLRJets", photons="AnaPhotons", met="AnaMET")

ROUTING = [
    ("EL_N 25000 >= 1",                     "CP::NObjectPtSelectorAlg",                    "NEL"),
    ("MU_N 25000 >= 1",                     "CP::NObjectPtSelectorAlg",                    "NMU"),
    ("JET_N 30000 >= 2",                    "CP::NObjectPtSelectorAlg",                    "NJET"),
    ("JET_N_BTAG >= 2",                     "CP::NObjectPtSelectorAlg",                    "NBJET"),
    ("JET_N_GHOST B >= 1",                  "CP::JetNGhostSelectorAlg",                    "NJETGHOST"),
    ("PH_N 25000 >= 1",                     "CP::NObjectPtSelectorAlg",                    "NPH"),
    ("TAU_N 25000 >= 1",                    "CP::NObjectPtSelectorAlg",                    "NTAU"),
    ("LJET_N 200000 >= 1",                  "CP::NObjectPtSelectorAlg",                    "NLJET"),
    ("LJET_N_GHOST B >= 1",                 "CP::JetNGhostSelectorAlg",                    "NLJETGHOST"),
    ("LJETMASS_N 50000 >= 1",               "CP::NObjectMassSelectorAlg",                  "NLJETMASS"),
    ("LJETMASSWINDOW_N 60000 100000 >= 1",  "CP::NLargeRJetMassWindowSelectorAlg",         "NLJETMASSWINDOW"),
    ("OBJ_N AnaJets 30000 >= 2",            "CP::NObjectPtSelectorAlg",                    "NOBJ"),
    ("SUM_EL_N_MU_N 25000 >= 2",            "CP::SumNLeptonPtSelectorAlg",                 "SUMNELNMU"),
    ("SUM_EL_N_MU_N_TAU_N 25000 >= 2",      "CP::SumNLeptonPtSelectorAlg",                 "SUMNLEPTONS"),
    ("MET > 30000",                         "CP::MissingETSelectorAlg",                    "MET"),
    ("MWT > 50000",                         "CP::TransverseMassSelectorAlg",               "MWT"),
    ("MET+MWT > 60000",                     "CP::MissingETPlusTransverseMassSelectorAlg",  "METMWT"),
    ("MLL > 80000",                         "CP::DileptonInvariantMassSelectorAlg",        "MLL_"),
    ("MLLWINDOW 80000 100000",              "CP::DileptonInvariantMassWindowSelectorAlg",  "MLLWINDOW"),
    ("MLL_OSSF 80000 100000",               "CP::DileptonOSSFInvariantMassWindowSelectorAlg", "MLL_OSSF"),
    ("OS",                                  "CP::ChargeSelectorAlg",                       "_OS_"),
    ("SS",                                  "CP::ChargeSelectorAlg",                       "_SS_"),
    ("RUN_NUMBER >= 300000",                "CP::RunNumberSelectorAlg",                    "RUN_NUMBER"),
]


class TestRouting(unittest.TestCase):
    def test_routing(self):
        for line, alg_type, name_sub in ROUTING:
            with self.subTest(line=line):
                sel = selectors(run(line, containers=ALL, btagDecoration="btag"))
                self.assertEqual(len(sel), 1, f"{line} -> {len(sel)} selectors")
                self.assertEqual(sel[0].getType(), alg_type)
                self.assertIn(name_sub, sel[0].getName())

    def test_btag_not_routed_as_plain_jet(self):
        algs = run("JET_N_BTAG >= 2", containers=ALL, btagDecoration="btag")
        self.assertTrue(named(algs, "NBJET"))
        self.assertFalse([a for a in algs if "NJET_" in a.getName()])

    def test_sum_tau_not_routed_as_sum(self):
        self.assertTrue(named(run("SUM_EL_N_MU_N_TAU_N 25000 >= 2", containers=ALL), "SUMNLEPTONS"))

    def test_metmwt_not_routed_as_met_or_mwt(self):
        sel = selectors(run("MET+MWT > 60000", containers=ALL))
        self.assertEqual(sel[0].getType(), "CP::MissingETPlusTransverseMassSelectorAlg")


class TestNonCuts(unittest.TestCase):
    def test_unknown_keyword_raises(self):
        self.assertRaises(ValueError, run, "NOT_A_KEYWORD 1 2 3", containers=ALL)

    def test_comment_and_blank_skipped(self):
        algs = selectors(run("# a comment\n\n   \nJET_N 30000 >= 2", containers=ALL))
        self.assertEqual(len(algs), 1)


if __name__ == "__main__":
    unittest.main()
