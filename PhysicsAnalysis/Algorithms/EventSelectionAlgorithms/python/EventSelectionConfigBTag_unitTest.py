#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Unit tests for JET_N_BTAG: default WP, custom btagger:WP, extra selection."""

import unittest
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import run, named, prop

BTAG = "ftag_select_GN2v01_FixedCutBEff_77"


def _jets(**kw):
    base = {"jets": "AnaJets"}
    base.update(kw)
    return base


class TestBTag(unittest.TestCase):
    def test_default_btag_3arg(self):
        algs = named(run("JET_N_BTAG >= 2", containers=_jets(), btagDecoration=BTAG), "NBJET")
        self.assertEqual(len(algs), 1)
        self.assertIn(BTAG, prop(algs[0], "objectSelection"))
        self.assertEqual(prop(algs[0], "sign"), "GE")
        self.assertEqual(prop(algs[0], "count"), 2)

    def test_default_btag_with_base_selection(self):
        algs = named(run("JET_N_BTAG >= 2",
                         containers={"jets": "AnaJets.baseline"}, btagDecoration=BTAG), "NBJET")
        sel = prop(algs[0], "objectSelection")
        self.assertIn("baseline", sel)
        self.assertIn(BTAG, sel)

    def test_custom_btagger_4arg(self):
        algs = named(run("JET_N_BTAG GN2v01:FixedCutBEff_85 >= 1",
                         containers=_jets(), btagDecoration=BTAG), "NBJET")
        self.assertIn("ftag_select_GN2v01_FixedCutBEff_85", prop(algs[0], "objectSelection"))

    def test_extra_selection_4arg(self):
        algs = named(run("JET_N_BTAG centraljet >= 1",
                         containers=_jets(), btagDecoration=BTAG), "NBJET")
        sel = prop(algs[0], "objectSelection")
        self.assertIn(BTAG, sel)
        self.assertIn("centraljet", sel)

    def test_extra_selection_plus_custom_btagger_5arg(self):
        algs = named(run("JET_N_BTAG centraljet GN2v01:FixedCutBEff_85 >= 1",
                         containers=_jets(), btagDecoration=BTAG), "NBJET")
        sel = prop(algs[0], "objectSelection")
        self.assertIn("ftag_select_GN2v01_FixedCutBEff_85", sel)
        self.assertIn("centraljet", sel)

    def test_missing_jets_raises(self):
        self.assertRaises(ValueError, run, "JET_N_BTAG >= 2", btagDecoration=BTAG)

    def test_wrong_argcount_raises(self):
        self.assertRaises(ValueError, run, "JET_N_BTAG a b c d e",
                          containers=_jets(), btagDecoration=BTAG)

    def test_malformed_btagger_raises(self):
        self.assertRaises(ValueError, run, "JET_N_BTAG GN2v01:WP:extra >= 1",
                          containers=_jets(), btagDecoration=BTAG)


if __name__ == "__main__":
    unittest.main()
