#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Unit tests for the EVENTVAR keyword: cut on an EventInfo scalar decoration
(e.g. a DNN/BDT discriminant) with a configurable stored type.

Formalism: EVENTVAR <type> <varname> <sign> <threshold>, where <varname> is
bare and the '_%SYS%' suffix is appended by the configuration."""

import unittest
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import run, named, prop


class TestEventVar(unittest.TestCase):
    def test_float_default(self):
        alg = named(run("EVENTVAR float dnn_score > 0.5"), "EVENTVAR")[0]
        self.assertEqual(alg.getType(), "CP::EventScalarSelectorAlg")
        self.assertEqual(prop(alg, "floatVariable"), "dnn_score_%SYS%")
        self.assertEqual(prop(alg, "sign"), "GT")
        self.assertEqual(prop(alg, "refValue"), 0.5)

    def test_explicit_int(self):
        alg = named(run("EVENTVAR int n_jets >= 5"), "EVENTVAR")[0]
        self.assertEqual(prop(alg, "intVariable"), "n_jets_%SYS%")
        self.assertEqual(prop(alg, "floatVariable", ""), "")
        self.assertEqual(prop(alg, "sign"), "GE")

    def test_explicit_double(self):
        alg = named(run("EVENTVAR double ht > 300000.0"), "EVENTVAR")[0]
        self.assertEqual(prop(alg, "doubleVariable"), "ht_%SYS%")

    def test_negative_threshold(self):
        alg = named(run("EVENTVAR float bdt_score > -0.2"), "EVENTVAR")[0]
        self.assertEqual(prop(alg, "floatVariable"), "bdt_score_%SYS%")
        self.assertEqual(prop(alg, "refValue"), -0.2)

    def test_all_signs(self):
        for sym, enum in [("<", "LT"), (">", "GT"), ("==", "EQ"), (">=", "GE"), ("<=", "LE")]:
            with self.subTest(sign=sym):
                alg = named(run(f"EVENTVAR float score {sym} 0.5"), "EVENTVAR")[0]
                self.assertEqual(prop(alg, "sign"), enum)

    def test_bad_type_raises(self):
        self.assertRaises(ValueError, run, "EVENTVAR complex score > 0.5")

    def test_bad_argcount_raises(self):
        self.assertRaises(ValueError, run, "EVENTVAR float score > 0.5 extra")
        self.assertRaises(ValueError, run, "EVENTVAR score > 0.5")

    def test_chains_after_other_cuts(self):
        algs = run("JET_N 30000 >= 2\nEVENTVAR float dnn > 0.8",
                   containers={"jets": "AnaJets"})
        ev = [a for a in algs if a.getType() == "CP::EventScalarSelectorAlg"][0]
        self.assertIn("NJET_1", prop(ev, "eventPreselection"))


if __name__ == "__main__":
    unittest.main()
