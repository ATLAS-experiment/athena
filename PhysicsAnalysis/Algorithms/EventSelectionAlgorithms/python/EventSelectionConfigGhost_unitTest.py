#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Unit tests for JET_N_GHOST and LJET_N_GHOST: ghost mapping, veto, pT form."""

import unittest
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import run, named, prop


class TestGhost(unittest.TestCase):
    def test_ghost_basic(self):
        for kw, opt, val, tag in [("JET_N_GHOST", "jets", "AnaJets", "NJETGHOST"),
                                  ("LJET_N_GHOST", "largeRjets", "AnaLRJets", "NLJETGHOST")]:
            with self.subTest(kw=kw):
                algs = named(run(f"{kw} B >= 1", containers={opt: val}), tag)
                self.assertEqual(len(algs), 1)
                self.assertEqual(algs[0].getType(), "CP::JetNGhostSelectorAlg")
                self.assertEqual(prop(algs[0], "ghost"), "GhostBHadronsFinalCount")
                self.assertEqual(prop(algs[0], "count"), 1)

    def test_ghost_with_veto(self):
        algs = named(run("JET_N_GHOST B!C >= 1", containers={"jets": "AnaJets"}), "NJETGHOST")
        self.assertEqual(prop(algs[0], "ghost"), "GhostBHadronsFinalCount")
        self.assertEqual(prop(algs[0], "veto"), "GhostCHadronsFinalCount")

    def test_ghost_with_minpt_5arg(self):
        algs = named(run("JET_N_GHOST B 30000 >= 1", containers={"jets": "AnaJets"}), "NJETGHOST")
        self.assertEqual(prop(algs[0], "minPt"), 30000.0)
        self.assertEqual(prop(algs[0], "count"), 1)

    def test_ghost_wrong_argcount(self):
        self.assertRaises(ValueError, run, "JET_N_GHOST B 30000 >= 1 extra",
                          containers={"jets": "AnaJets"})


if __name__ == "__main__":
    unittest.main()
